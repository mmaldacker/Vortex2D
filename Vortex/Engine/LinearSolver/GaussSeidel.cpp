//
//  GaussSeidel.cpp
//  Vortex
//

#include "GaussSeidel.h"
#include <cmath>
#include <glm/gtc/constants.hpp>

#include "vortex_generated_spirv.h"

namespace Vortex
{
namespace Fluid
{
GaussSeidel::GaussSeidel(Renderer::Device& device, const glm::ivec2& size)
    : mW(2.0f / (1.0f + std::sin(glm::pi<float>() / std::sqrt((float)(size.x * size.y)))))
    , mPreconditionerIterations(1)
    , mError(device, size)
    , mGaussSeidel(device, Renderer::MakeCheckerboardComputeSize(size), SPIRV::GaussSeidel_comp)
    , mInitCmd(device, false)
    , mGaussSeidelCmd(device, false)
{
}

GaussSeidel::~GaussSeidel() {}

void GaussSeidel::SetW(float w)
{
  mW = w;
}

void GaussSeidel::SetPreconditionerIterations(int iterations)
{
  mPreconditionerIterations = iterations;
}

void GaussSeidel::Bind(Renderer::GenericBuffer& d,
                       Renderer::GenericBuffer& l,
                       Renderer::GenericBuffer& div,
                       Renderer::GenericBuffer& pressure)
{
  mPressure = &pressure;

  mError.Bind(d, l, div, pressure);
  mGaussSeidelBound = mGaussSeidel.Bind({pressure, d, l, div});

  mInitCmd.Record([&](Renderer::CommandEncoder& command) { pressure.Clear(command); });
  mGaussSeidelCmd.Record([&](Renderer::CommandEncoder& command) { Record(command, 1); });
}

void GaussSeidel::BindRigidbody(float /*delta*/,
                                Renderer::GenericBuffer& /*d*/,
                                RigidBody& /*rigidBody*/)
{
}

void GaussSeidel::Solve(Parameters& params, const std::vector<RigidBody*>& /*rigidbodies*/)
{
  params.Reset();

  mInitCmd.Submit();

  if (params.Type == Parameters::SolverType::Iterative)
  {
    params.OutError = mError.Submit().Wait().GetError();
    if (params.OutError <= params.ErrorTolerance)
    {
      return;
    }

    mError.Submit();
  }

  auto initialError = params.OutError;
  for (unsigned i = 0; !params.IsFinished(initialError); params.OutIterations = ++i)
  {
    mGaussSeidelCmd.Submit();

    if (params.Type == Parameters::SolverType::Iterative)
    {
      params.OutError = mError.Wait().GetError();
      mError.Submit();
    }
  }

  if (params.Type == Parameters::SolverType::Iterative)
  {
    mError.Wait();
  }
}

float GaussSeidel::GetError()
{
  return mError.Submit().Wait().GetError();
}

void GaussSeidel::Record(Renderer::CommandEncoder& command)
{
  assert(mPressure != nullptr);
  Record(command, mPreconditionerIterations);
}

void GaussSeidel::Record(Renderer::CommandEncoder& command, int iterations)
{
  mPressure->Barrier(command, Renderer::Access::Write, Renderer::Access::Write);

  for (int i = 0; i < iterations; ++i)
  {
    mGaussSeidelBound.PushConstant(command, mW, 1);
    mGaussSeidelBound.Record(command);
    mPressure->Barrier(command, Renderer::Access::Write, Renderer::Access::Write);
    mGaussSeidelBound.PushConstant(command, mW, 0);
    mGaussSeidelBound.Record(command);
    mPressure->Barrier(command, Renderer::Access::Write, Renderer::Access::Write);
  }
}

Renderer::ComputeSize MakeLocalSize(const glm::ivec2& size)
{
  Renderer::ComputeSize computeSize(size);
  computeSize.WorkSize = glm::ivec2(1);
  computeSize.LocalSize = glm::ivec2(16);  // TODO shouldn't be hardcoded 16

  return computeSize;
}

LocalGaussSeidel::LocalGaussSeidel(Renderer::Device& device, const glm::ivec2& size)
    : mLocalGaussSeidel(device, MakeLocalSize(size), SPIRV::LocalGaussSeidel_comp)
{
  // TODO check size is within local size
}

LocalGaussSeidel::~LocalGaussSeidel() {}

void LocalGaussSeidel::Bind(Renderer::GenericBuffer& d,
                            Renderer::GenericBuffer& l,
                            Renderer::GenericBuffer& div,
                            Renderer::GenericBuffer& pressure)
{
  mPressure = &pressure;
  mLocalGaussSeidelBound = mLocalGaussSeidel.Bind({pressure, d, l, div});
}

void LocalGaussSeidel::Record(Renderer::CommandEncoder& command)
{
  assert(mPressure != nullptr);
  mLocalGaussSeidelBound.Record(command);
  mPressure->Barrier(command, Renderer::Access::Write, Renderer::Access::Read);
}

}  // namespace Fluid
}  // namespace Vortex
