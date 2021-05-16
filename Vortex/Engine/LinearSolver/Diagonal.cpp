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
Diagonal::Diagonal(const Renderer::Device& device, const glm::ivec2& size)
    : mDiagonal(device, size, SPIRV::Diagonal_comp)
{
}

Diagonal::~Diagonal() {}

void Diagonal::Bind(Renderer::GenericBuffer& d,
                    Renderer::GenericBuffer& /*l*/,
                    Renderer::GenericBuffer& b,
                    Renderer::GenericBuffer& pressure)
{
  mDiagonalBound = mDiagonal.Bind({d, b, pressure});
}

void Diagonal::Record(vk::CommandBuffer commandBuffer)
{
  mDiagonalBound.Record(commandBuffer);
}

}  // namespace Fluid
}  // namespace Vortex
