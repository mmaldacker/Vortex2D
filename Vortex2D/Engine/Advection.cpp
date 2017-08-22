//
//  Advection.cpp
//  Vortex
//

#include "Advection.h"

namespace Vortex2D { namespace Fluid {

Advection::Advection(const Renderer::Device& device, const glm::ivec2& size, float dt, Renderer::Texture& velocity)
    : mVelocity(velocity)
    , mVelocityAdvect(device, size, "../Vortex2D/AdvectVelocity.comp.spv", {vk::DescriptorType::eStorageImage}, 4)
    , mVelocityAdvectBound(mVelocityAdvect.Bind({velocity}))
    , mAdvect(device, size, "../Vortex2D/Advect.comp.spv", {vk::DescriptorType::eStorageImage,
                                                            vk::DescriptorType::eStorageImage}, 4)
    , mAdvecVelocityCmd(device, false)
{
    mAdvecVelocityCmd.Record([&](vk::CommandBuffer commandBuffer)
    {
        mVelocityAdvectBound.PushConstant(commandBuffer, 8, dt);
        mVelocityAdvectBound.Record(commandBuffer);
        velocity.Barrier(commandBuffer,
                         vk::ImageLayout::eGeneral,
                         vk::AccessFlagBits::eShaderWrite,
                         vk::ImageLayout::eGeneral,
                         vk::AccessFlagBits::eShaderRead);
    });
}

void Advection::Advect()
{
    mAdvecVelocityCmd.Submit();
}

void Advection::Advect(Renderer::Buffer& buffer)
{
    /*
    buffer.Swap();
    buffer = mAdvect(Back(buffer), mVelocity);
    */
}

}}
