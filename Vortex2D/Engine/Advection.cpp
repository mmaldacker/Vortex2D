//
//  Advection.cpp
//  Vortex
//

#include "Advection.h"

#include <Vortex2D/Renderer/Pipeline.h>

namespace Vortex2D { namespace Fluid {

Advection::Advection(const Renderer::Device& device, const glm::ivec2& size, float dt, Renderer::Texture& velocity)
    : mDt(dt)
    , mVelocity(velocity)
    , mTmpVelocity(device, size.x, size.y, vk::Format::eR32G32Sfloat, false)
    , mField(device, size.x, size.y, vk::Format::eB8G8R8A8Unorm, false)
    , mVelocityAdvect(device, size, "../Vortex2D/AdvectVelocity.comp.spv",
                     {vk::DescriptorType::eStorageImage,
                      vk::DescriptorType::eStorageImage},
                      Renderer::PushConstantsSize<float>())
    , mVelocityAdvectBound(mVelocityAdvect.Bind({velocity, mTmpVelocity}))
    , mAdvect(device, size, "../Vortex2D/Advect.comp.spv",
             {vk::DescriptorType::eStorageImage,
              vk::DescriptorType::eStorageImage,
              vk::DescriptorType::eStorageImage},
              Renderer::PushConstantsSize<float>())
    , mAdvectVelocityCmd(device, false)
    , mAdvectCmd(device, false)
{
    mAdvectVelocityCmd.Record([&](vk::CommandBuffer commandBuffer)
    {
        mVelocityAdvectBound.PushConstant(commandBuffer, 8, dt);
        mVelocityAdvectBound.Record(commandBuffer);
        mTmpVelocity.Barrier(commandBuffer,
                          vk::ImageLayout::eGeneral,
                          vk::AccessFlagBits::eShaderWrite,
                          vk::ImageLayout::eGeneral,
                          vk::AccessFlagBits::eShaderRead);
        velocity.CopyFrom(commandBuffer, mTmpVelocity);
    });
}

void Advection::AdvectVelocity()
{
    mAdvectVelocityCmd.Submit();
}

void Advection::AdvectInit(Renderer::Texture& field)
{
    mAdvectBound = mAdvect.Bind({mVelocity, field, mField});
    mAdvectCmd.Record([&](vk::CommandBuffer commandBuffer)
    {
        mAdvectBound.PushConstant(commandBuffer, 8, mDt);
        mAdvectBound.Record(commandBuffer);
        mField.Barrier(commandBuffer,
                      vk::ImageLayout::eGeneral,
                      vk::AccessFlagBits::eShaderWrite,
                      vk::ImageLayout::eGeneral,
                      vk::AccessFlagBits::eShaderRead);
        field.CopyFrom(commandBuffer, mField);
    });
}

void Advection::Advect()
{
    mAdvectCmd.Submit();
}

}}
