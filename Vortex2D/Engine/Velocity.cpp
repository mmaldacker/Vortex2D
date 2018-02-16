//
//  Velocity.cpp
//  Vortex2D
//

#include "Velocity.h"

namespace Vortex2D { namespace Fluid {

Velocity::Velocity(const Renderer::Device& device, const glm::ivec2& size)
    : mInputVelocity(device, size.x, size.y, vk::Format::eR32G32Sfloat)
    , mOutputVelocity(device, size.x, size.y, vk::Format::eR32G32Sfloat)
{

}

Renderer::RenderTexture& Velocity::Input()
{
    return mInputVelocity;
}

Renderer::Texture& Velocity::Output()
{
    return mOutputVelocity;
}

void Velocity::CopyBack(vk::CommandBuffer commandBuffer)
{
    mInputVelocity.Barrier(commandBuffer,
                           vk::ImageLayout::eGeneral, vk::AccessFlagBits::eShaderWrite,
                           vk::ImageLayout::eGeneral, vk::AccessFlagBits::eShaderRead);
    mInputVelocity.CopyFrom(commandBuffer, mOutputVelocity);
}

void Velocity::Clear(vk::CommandBuffer commandBuffer)
{
    mInputVelocity.Clear(commandBuffer, std::array<float, 4>{0.0f, 0.0f, 0.0f, 0.0f});
}

}}
