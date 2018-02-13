//
//  Diagonal.cpp
//  Vortex2D
//

#include "Diagonal.h"

#include "vortex2d_generated_spirv.h"

namespace Vortex2D { namespace Fluid {

Diagonal::Diagonal(const Renderer::Device& device, const glm::ivec2& size)
    : mDiagonal(device, size, Diagonal_comp)
{

}

void Diagonal::Init(Renderer::GenericBuffer& d,
                    Renderer::GenericBuffer& l,
                    Renderer::GenericBuffer& b,
                    Renderer::GenericBuffer& pressure)
{
    mDiagonalBound = mDiagonal.Bind({d, b, pressure});
}

void Diagonal::Record(vk::CommandBuffer commandBuffer)
{
    mDiagonalBound.Record(commandBuffer);
}

}}
