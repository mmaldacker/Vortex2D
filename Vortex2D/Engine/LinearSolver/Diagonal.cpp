//
//  Diagonal.cpp
//  Vortex2D
//

#include "Diagonal.h"

namespace Vortex2D { namespace Fluid {

Diagonal::Diagonal(const Renderer::Device& device, const glm::ivec2& size)
    : mDiagonal(device, size, "../Vortex2D/Diagonal.comp.spv",
                {vk::DescriptorType::eStorageBuffer,
                vk::DescriptorType::eStorageBuffer,
                vk::DescriptorType::eStorageBuffer})
{

}

void Diagonal::Init(Renderer::Buffer& A,
          Renderer::Buffer& b,
          Renderer::Buffer& pressure,
          Renderer::Work& buildMatrix,
          Renderer::Texture& solidPhi,
          Renderer::Texture& liquidPhi)
{
    mDiagonalBound = mDiagonal.Bind({A, b, pressure});
}

void Diagonal::RecordInit(vk::CommandBuffer commandBuffer)
{

}

void Diagonal::Record(vk::CommandBuffer commandBuffer)
{
    mDiagonalBound.Record(commandBuffer);
}

}}
