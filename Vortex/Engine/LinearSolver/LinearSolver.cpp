//
//  LinearSolver.cpp
//  Vortex
//

#include "LinearSolver.h"

#include <Vortex/Renderer/CommandBuffer.h>
#include "vortex_generated_spirv.h"

namespace Vortex
{
namespace Fluid
{
LinearSolver::Parameters::Parameters(SolverType type, unsigned iterations, float errorTolerance)
    : Type(type)
    , Iterations(iterations)
    , ErrorTolerance(errorTolerance)
    , OutIterations(0)
    , OutError(0.0f)
{
}

LinearSolver::Data::Data(Renderer::Device& device,
                         const glm::ivec2& size,
                         Renderer::MemoryUsage memoryUsage)
    : Diagonal(device, size.x * size.y, memoryUsage)
    , Lower(device, size.x * size.y, memoryUsage)
    , B(device, size.x * size.y, memoryUsage)
    , X(device, size.x * size.y, memoryUsage)
{
  device.Execute(
      [&](Renderer::CommandEncoder& command)
      {
        Diagonal.Clear(command);
        Lower.Clear(command);
        B.Clear(command);
        X.Clear(command);
      });
}

LinearSolver::DebugData::DebugData(Renderer::Device& device, const glm::ivec2& size)
    : Diagonal(device, size.x, size.y, Renderer::Format::R32Sfloat)
    , Lower(device, size.x, size.y, Renderer::Format::R32G32Sfloat)
    , B(device, size.x, size.y, Renderer::Format::R32Sfloat)
    , X(device, size.x, size.y, Renderer::Format::R32Sfloat)
{
}

LinearSolver::DebugCopy::DebugCopy(Renderer::Device& device,
                                   const glm::ivec2& size,
                                   LinearSolver::Data& data,
                                   LinearSolver::DebugData& debugData)
    : mDebugDataCopy(device, Renderer::ComputeSize{size}, SPIRV::DebugDataCopy_comp)
    , mDebugDataCopyBound(mDebugDataCopy.Bind({data.Diagonal,
                                               data.Lower,
                                               data.X,
                                               data.B,
                                               debugData.Diagonal,
                                               debugData.Lower,
                                               debugData.X,
                                               debugData.B}))
    , mCopy(device, false)
{
  mCopy.Record(
      [&](Renderer::CommandEncoder& command)
      {
        command.DebugMarkerBegin("Debug data copy", {0.30f, 0.01f, 0.19f, 1.0f});
        mDebugDataCopyBound.Record(command);
        command.DebugMarkerEnd();
      });
}

void LinearSolver::DebugCopy::Copy()
{
  mCopy.Submit();
}

bool LinearSolver::Parameters::IsFinished(float initialError) const
{
  if (Type == SolverType::Fixed)
  {
    return OutIterations >= Iterations;
  }

  if (Iterations > 0)
  {
    return OutIterations >= Iterations || OutError <= ErrorTolerance * initialError;
  }
  else
  {
    return OutError <= ErrorTolerance;
  }
}

void LinearSolver::Parameters::Reset()
{
  OutError = 0.0f;
  OutIterations = 0;
}

LinearSolver::Parameters FixedParams(unsigned iterations)
{
  return LinearSolver::Parameters(LinearSolver::Parameters::SolverType::Fixed, iterations);
}

LinearSolver::Parameters IterativeParams(float errorTolerance)
{
  return LinearSolver::Parameters(
      LinearSolver::Parameters::SolverType::Iterative, 1000, errorTolerance);
}

LinearSolver::Error::Error(Renderer::Device& device, const glm::ivec2& size)
    : mResidual(device, size.x * size.y)
    , mError(device)
    , mLocalError(device, 1, Renderer::MemoryUsage::GpuToCpu)
    , mResidualWork(device, Renderer::ComputeSize{size}, SPIRV::Residual_comp)
    , mReduceMax(device, size.x * size.y)
    , mReduceMaxBound(mReduceMax.Bind(mResidual, mError))
    , mErrorCmd(device)
{
  device.Execute([&](Renderer::CommandEncoder& command) { mResidual.Clear(command); });
}

void LinearSolver::Error::Bind(Renderer::GenericBuffer& d,
                               Renderer::GenericBuffer& l,
                               Renderer::GenericBuffer& div,
                               Renderer::GenericBuffer& pressure)
{
  mResidualBound = mResidualWork.Bind({pressure, d, l, div, mResidual});

  mErrorCmd.Record(
      [&](Renderer::CommandEncoder& command)
      {
        mResidualBound.Record(command);
        mResidual.Barrier(command, Renderer::Access::Write, Renderer::Access::Read);

        mReduceMaxBound.Record(command);
        mError.Barrier(command, Renderer::Access::Write, Renderer::Access::Read);

        mLocalError.CopyFrom(command, mError);
      });
}

LinearSolver::Error& LinearSolver::Error::Submit()
{
  mErrorCmd.Submit();
  return *this;
}

LinearSolver::Error& LinearSolver::Error::Wait()
{
  mErrorCmd.Wait();
  return *this;
}

float LinearSolver::Error::GetError()
{
  float error;
  Renderer::CopyTo(mLocalError, error);
  return error;
}

}  // namespace Fluid
}  // namespace Vortex
