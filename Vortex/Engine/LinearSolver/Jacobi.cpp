//
//  Jacobi.cpp
//  Vortex
//

#include "Jacobi.h"
#include <cmath>
#include <glm/gtc/constants.hpp>

#include "vortex_generated_spirv.h"

namespace Vortex
{
namespace Fluid
{
Jacobi::Jacobi(Renderer::Device& device, const glm::ivec2& size)
    : mW(1.0f)
    , mPreconditionerIterations(1)
    , mBackPressure(device, size.x * size.y)
    , mJacobi(device, Renderer::ComputeSize{size}, SPIRV::DampedJacobi_comp)
{
}

void Jacobi::SetW(float w)
{
  mW = w;
}

void Jacobi::SetPreconditionerIterations(int iterations)
{
  mPreconditionerIterations = iterations;
}

void Jacobi::Bind(Renderer::GenericBuffer& d,
                  Renderer::GenericBuffer& l,
                  Renderer::GenericBuffer& div,
                  Renderer::GenericBuffer& pressure)
{
  mPressure = &pressure;
  mJacobiFrontBound = mJacobi.Bind({pressure, mBackPressure, d, l, div});
  mJacobiBackBound = mJacobi.Bind({mBackPressure, pressure, d, l, div});
}

void Jacobi::Record(Renderer::CommandEncoder& command)
{
  assert(mPressure != nullptr);
  Record(command, mPreconditionerIterations);
}

void Jacobi::Record(Renderer::CommandEncoder& command, int iterations)
{
  mBackPressure.Clear(command);

  for (int i = 0; i < iterations; i++)
  {
    mJacobiFrontBound.PushConstant(command, mW);
    mJacobiFrontBound.Record(command);
    mBackPressure.Barrier(command, Renderer::Access::Write, Renderer::Access::Read);
    mJacobiBackBound.PushConstant(command, mW);
    mJacobiBackBound.Record(command);
    mPressure->Barrier(command, Renderer::Access::Write, Renderer::Access::Read);
  }
}

}  // namespace Fluid
}  // namespace Vortex
