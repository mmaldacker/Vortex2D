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
    , mJacobi(device, size, SPIRV::DampedJacobi_comp)
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

void Jacobi::Record(vk::CommandBuffer commandBuffer)
{
  assert(mPressure != nullptr);
  Record(commandBuffer, mPreconditionerIterations);
}

void Jacobi::Record(vk::CommandBuffer commandBuffer, int iterations)
{
  mBackPressure.Clear(commandBuffer);
  mBackPressure.Barrier(
      commandBuffer, vk::AccessFlagBits::eMemoryWrite, vk::AccessFlagBits::eShaderWrite);

  for (int i = 0; i < iterations; i++)
  {
    mJacobiFrontBound.PushConstant(commandBuffer, mW);
    mJacobiFrontBound.Record(commandBuffer);
    mBackPressure.Barrier(
        commandBuffer, vk::AccessFlagBits::eShaderWrite, vk::AccessFlagBits::eShaderRead);
    mJacobiBackBound.PushConstant(commandBuffer, mW);
    mJacobiBackBound.Record(commandBuffer);
    mPressure->Barrier(
        commandBuffer, vk::AccessFlagBits::eShaderWrite, vk::AccessFlagBits::eShaderRead);
  }
}

}  // namespace Fluid
}  // namespace Vortex
