//
//  Multigrid.cpp
//  Vortex
//

#include "Multigrid.h"

#include <Vortex/Engine/Pressure.h>

#include "vortex_generated_spirv.h"

namespace Vortex
{
namespace Fluid
{
Depth::Depth(const glm::ivec2& size)
{
  auto s = size;
  mDepths.push_back(s);

  const float min_size = 16.0f;
  while (s.x > min_size && s.y > min_size)
  {
    if (s.x % 2 != 0 || s.y % 2 != 0)
      throw std::runtime_error("Invalid multigrid size");

    s = s / glm::ivec2(2);
    mDepths.push_back(s);
  }
}

int Depth::GetMaxDepth() const
{
  return static_cast<int>(mDepths.size() - 1);
}

glm::ivec2 Depth::GetDepthSize(std::size_t i) const
{
  assert(i < mDepths.size());
  return mDepths[i];
}

std::unique_ptr<Preconditioner> MakeSmoother(Renderer::Device& device,
                                             glm::ivec2 size,
                                             Multigrid::SmootherSolver smoother,
                                             int numSmoothingIterations)
{
  if (smoother == Multigrid::SmootherSolver::Jacobi)
  {
    auto solver = std::make_unique<Jacobi>(device, size);
    solver->SetPreconditionerIterations(numSmoothingIterations);
    solver->SetW(2.0f / 3.0f);

    return std::move(solver);
  }
  else if (smoother == Multigrid::SmootherSolver::GaussSeidel)
  {
    auto solver = std::make_unique<GaussSeidel>(device, size);
    solver->SetPreconditionerIterations(numSmoothingIterations);
    solver->SetW(2.0f / 3.0f);

    return std::move(solver);
  }

  return {};
}

Multigrid::Multigrid(Renderer::Device& device,
                     const glm::ivec2& size,
                     float delta,
                     int numSmoothingIterations,
                     SmootherSolver smoother)
    : mDevice(device)
    , mDepth(size)
    , mDelta(delta)
    , mNumSmoothingIterations(numSmoothingIterations)
    , mResidualWork(device, Renderer::ComputeSize{size}, SPIRV::Residual_comp)
    , mTransfer(device)
    , mPhiScaleWork(device, Renderer::ComputeSize{size}, SPIRV::PhiScale_comp)
    , mSmoother(device, mDepth.GetDepthSize(mDepth.GetMaxDepth()))
    , mBuildHierarchies(device, false)
    , mFullCycleSolver(device, false)
    , mVCycleSolver(device, false)
    , mError(device, size)
{
  for (int i = 1; i <= mDepth.GetMaxDepth(); i++)
  {
    auto s = mDepth.GetDepthSize(i);
    mDatas.emplace_back(device, s);

    mSolidPhis.emplace_back(device, s);
    mLiquidPhis.emplace_back(device, s);
  }

  for (int i = 0; i < mDepth.GetMaxDepth(); i++)
  {
    auto s = mDepth.GetDepthSize(i);
    mResiduals.emplace_back(device, s.x * s.y);
    mSmoothers.emplace_back(MakeSmoother(device, s, smoother, numSmoothingIterations));
  }

  int depth = mDepth.GetMaxDepth() - 1;
  mSmoother.Bind(mDatas[depth].Diagonal, mDatas[depth].Lower, mDatas[depth].B, mDatas[depth].X);
  mResidualWorkBound.resize(mDepth.GetMaxDepth() + 1);
}

Multigrid::~Multigrid() {}

void Multigrid::Bind(Renderer::GenericBuffer& d,
                     Renderer::GenericBuffer& l,
                     Renderer::GenericBuffer& b,
                     Renderer::GenericBuffer& pressure)

{
  mPressure = &pressure;

  mResidualWorkBound[0] = mResidualWork.Bind({pressure, d, l, b, mResiduals[0]});
  mSmoothers[0]->Bind(d, l, b, pressure);

  auto s = mDepth.GetDepthSize(0);
  mTransfer.RestrictBind(0, s, mResiduals[0], d, mDatas[0].B, mDatas[0].Diagonal);
  mTransfer.ProlongateBind(0, s, pressure, d, mDatas[0].X, mDatas[0].Diagonal);

  mFullCycleSolver.Record(
      [&](Renderer::CommandEncoder& command)
      {
        pressure.Clear(command);
        RecordFullCycle(command);
      });

  mVCycleSolver.Record([&](Renderer::CommandEncoder& command) { RecordVCycle(command, 0); });

  mError.Bind(d, l, b, pressure);
}

void Multigrid::BuildHierarchiesBind(Pressure& pressure,
                                     Renderer::Texture& solidPhi,
                                     Renderer::Texture& liquidPhi)
{
  auto s = mDepth.GetDepthSize(1);
  mLiquidPhiScaleWorkBound.push_back(
      mPhiScaleWork.Bind(Renderer::ComputeSize{s}, {liquidPhi, mLiquidPhis[0]}));
  mSolidPhiScaleWorkBound.push_back(
      mPhiScaleWork.Bind(Renderer::ComputeSize{s}, {solidPhi, mSolidPhis[0]}));

  RecursiveBind(pressure, 1);

  mBuildHierarchies.Record(
      [&](Renderer::CommandEncoder& command)
      {
        command.DebugMarkerBegin("Build hierarchies", {0.36f, 0.85f, 0.55f, 1.0f});
        for (int i = 0; i < mDepth.GetMaxDepth(); i++)
        {
          mLiquidPhiScaleWorkBound[i].Record(command);
          mLiquidPhis[i].Barrier(command,
                                 Renderer::ImageLayout::General,
                                 Renderer::Access::Write,
                                 Renderer::ImageLayout::General,
                                 Renderer::Access::Read);

          mSolidPhiScaleWorkBound[i].Record(command);
          mSolidPhis[i].Barrier(command,
                                Renderer::ImageLayout::General,
                                Renderer::Access::Write,
                                Renderer::ImageLayout::General,
                                Renderer::Access::Read);

          mMatrixBuildBound[i].PushConstant(command, mDelta);
          mMatrixBuildBound[i].Record(command);
          mDatas[i].Diagonal.Barrier(command, Renderer::Access::Write, Renderer::Access::Read);
          mDatas[i].Lower.Barrier(command, Renderer::Access::Write, Renderer::Access::Read);
          mDatas[i].B.Clear(command);
        }

        int maxDepth = mDepth.GetMaxDepth();
        mMatrixBuildBound[maxDepth - 1].PushConstant(command, mDelta);
        mMatrixBuildBound[maxDepth - 1].Record(command);
        mDatas[maxDepth - 1].Diagonal.Barrier(
            command, Renderer::Access::Write, Renderer::Access::Read);
        mDatas[maxDepth - 1].Lower.Barrier(
            command, Renderer::Access::Write, Renderer::Access::Read);
        command.DebugMarkerEnd();
      });
}

void Multigrid::RecursiveBind(Pressure& pressure, std::size_t depth)
{
  auto s0 = mDepth.GetDepthSize(depth);

  if (static_cast<int32_t>(depth) < mDepth.GetMaxDepth())
  {
    auto s1 = mDepth.GetDepthSize(depth + 1);
    mLiquidPhiScaleWorkBound.push_back(mPhiScaleWork.Bind(
        Renderer::ComputeSize{s1}, {mLiquidPhis[depth - 1], mLiquidPhis[depth]}));
    mSolidPhiScaleWorkBound.push_back(
        mPhiScaleWork.Bind(Renderer::ComputeSize{s1}, {mSolidPhis[depth - 1], mSolidPhis[depth]}));

    mResidualWorkBound[depth] = mResidualWork.Bind(Renderer::ComputeSize{s0},
                                                   {mDatas[depth - 1].X,
                                                    mDatas[depth - 1].Diagonal,
                                                    mDatas[depth - 1].Lower,
                                                    mDatas[depth - 1].B,
                                                    mResiduals[depth]});

    mTransfer.RestrictBind(depth,
                           s0,
                           mResiduals[depth],
                           mDatas[depth - 1].Diagonal,
                           mDatas[depth].B,
                           mDatas[depth].Diagonal);

    mTransfer.ProlongateBind(depth,
                             s0,
                             mDatas[depth - 1].X,
                             mDatas[depth - 1].Diagonal,
                             mDatas[depth].X,
                             mDatas[depth].Diagonal);

    mSmoothers[depth]->Bind(mDatas[depth - 1].Diagonal,
                            mDatas[depth - 1].Lower,
                            mDatas[depth - 1].B,
                            mDatas[depth - 1].X);

    RecursiveBind(pressure, depth + 1);
  }

  mMatrixBuildBound.push_back(pressure.BindMatrixBuild(s0,
                                                       mDatas[depth - 1].Diagonal,
                                                       mDatas[depth - 1].Lower,
                                                       mLiquidPhis[depth - 1],
                                                       mSolidPhis[depth - 1]));
}

void Multigrid::BuildHierarchies()
{
  mBuildHierarchies.Submit();
}

void Multigrid::Smoother(Renderer::CommandEncoder& command, int n)
{
  mSmoothers[n]->Record(command);
}

void Multigrid::Record(Renderer::CommandEncoder& command)
{
  command.DebugMarkerBegin("Multigrid", {0.48f, 0.25f, 0.19f, 1.0f});
  assert(mPressure != nullptr);

  mPressure->Clear(command);

  RecordVCycle(command, 0);

  command.DebugMarkerEnd();
}

void Multigrid::BindRigidbody(float /*delta*/, Renderer::GenericBuffer& /*d*/, RigidBody& rigidBody)
{
  if (rigidBody.GetType() == RigidBody::Type::eStrong)
  {
    throw std::runtime_error("Strong coupling not supported for multigrid solver");
  }
}

void Multigrid::Solve(Parameters& params, const std::vector<RigidBody*>& /*rigidBodies*/)
{
  params.Reset();
  mFullCycleSolver.Submit();
  for (int i = 0; i < params.Iterations; i++)
  {
    mVCycleSolver.Submit();
  }
}

float Multigrid::GetError()
{
  return mError.Submit().Wait().GetError();
}

void Multigrid::RecordVCycle(Renderer::CommandEncoder& command, int depth)
{
  if (depth == mDepth.GetMaxDepth())
  {
    mSmoother.Record(command);
  }
  else
  {
    Smoother(command, depth);

    mResidualWorkBound[depth].Record(command);
    mResiduals[depth].Barrier(command, Renderer::Access::Write, Renderer::Access::Read);

    mTransfer.Restrict(command, depth);

    mDatas[depth].X.Clear(command);
    mDatas[depth].X.Barrier(command, Renderer::Access::Write, Renderer::Access::Write);

    RecordVCycle(command, depth + 1);

    mTransfer.Prolongate(command, depth);

    Smoother(command, depth);
  }
}

void Multigrid::RecordFullCycle(Renderer::CommandEncoder& command)
{
  mResidualWorkBound[0].Record(command);
  mResiduals[0].Barrier(command, Renderer::Access::Write, Renderer::Access::Read);

  for (int i = 0; i < mDepth.GetMaxDepth() - 1; i++)
  {
    mTransfer.Restrict(command, i);
    mResiduals[i + 1].CopyFrom(command, mDatas[i].B);
  }

  int depth = mDepth.GetMaxDepth() - 1;
  mDatas[depth].X.Clear(command);
  mDatas[depth].X.Barrier(command, Renderer::Access::Write, Renderer::Access::Write);
  mSmoother.Record(command);

  for (int i = depth; i >= 0; i--)
  {
    mTransfer.Prolongate(command, depth);
    RecordVCycle(command, i);
  }
}

}  // namespace Fluid
}  // namespace Vortex
