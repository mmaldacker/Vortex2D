//
//  Diagonal.cpp
//  Vortex
//

#include "Diagonal.h"

#include "vortex_generated_spirv.h"

namespace Vortex
{
namespace Fluid
{
Diagonal::Diagonal(Renderer::Device& device, const glm::ivec2& size)
    : mDiagonal(device, Renderer::ComputeSize{size}, SPIRV::Diagonal_comp)
{
}

Diagonal::~Diagonal() {}

void Diagonal::Bind(Renderer::GenericBuffer& d,
                    Renderer::GenericBuffer& /*l*/,
                    Renderer::GenericBuffer& b,
                    Renderer::GenericBuffer& pressure)
{
  mPressure = &pressure;
  mDiagonalBound = mDiagonal.Bind({d, b, pressure});
}

void Diagonal::Record(vk::CommandBuffer commandBuffer)
{
  assert(mPressure != nullptr);

  mDiagonalBound.Record(commandBuffer);
  mPressure->Barrier(
      commandBuffer, vk::AccessFlagBits::eShaderWrite, vk::AccessFlagBits::eShaderRead);
}

}  // namespace Fluid
}  // namespace Vortex
