//
//  Extrapolation.cpp
//  Vortex2D

#include "Extrapolation.h"

namespace Vortex2D { namespace Fluid {

Extrapolation::Extrapolation(const Renderer::Device& device,
                             const glm::ivec2& size,
                             Renderer::GenericBuffer& valid,
                             Renderer::Texture& velocity,
                             Renderer::Texture& solidVelocity,
                             Renderer::Texture& solidPhi)
    : mValid(device,size.x*size.y)
    , mVelocity(device, size.x, size.y, vk::Format::eR32G32Sfloat, false)
    , mExtrapolateVelocity(device, size, "../Vortex2D/ExtrapolateVelocity.comp.spv")
    , mExtrapolateVelocityFrontBound(mExtrapolateVelocity.Bind({valid, mValid, velocity}))
    , mExtrapolateVelocityBackBound(mExtrapolateVelocity.Bind({mValid, valid, velocity}))
    , mConstrainVelocity(device, size, "../Vortex2D/ConstrainVelocity.comp.spv")
    , mConstrainVelocityBound(mConstrainVelocity.Bind({solidPhi, velocity, solidVelocity, mVelocity}))
    , mExtrapolateCmd(device, false)
    , mConstrainCmd(device, false)
{
    mExtrapolateCmd.Record([&](vk::CommandBuffer commandBuffer)
    {
        const int layers = 10;
        for (int i = 0; i < layers / 2; i++)
        {
            mExtrapolateVelocityFrontBound.Record(commandBuffer);
            velocity.Barrier(commandBuffer,
                             vk::ImageLayout::eGeneral,
                             vk::AccessFlagBits::eShaderWrite,
                             vk::ImageLayout::eGeneral,
                             vk::AccessFlagBits::eShaderRead);
            mValid.Barrier(commandBuffer,
                           vk::AccessFlagBits::eShaderWrite,
                           vk::AccessFlagBits::eShaderRead);
            mExtrapolateVelocityBackBound.Record(commandBuffer);
            velocity.Barrier(commandBuffer,
                             vk::ImageLayout::eGeneral,
                             vk::AccessFlagBits::eShaderWrite,
                             vk::ImageLayout::eGeneral,
                             vk::AccessFlagBits::eShaderRead);
            valid.Barrier(commandBuffer,
                          vk::AccessFlagBits::eShaderWrite,
                          vk::AccessFlagBits::eShaderRead);
        }
    });

    mConstrainCmd.Record([&](vk::CommandBuffer commandBuffer)
    {
        mConstrainVelocityBound.Record(commandBuffer);
        mVelocity.Barrier(commandBuffer,
                          vk::ImageLayout::eGeneral,
                          vk::AccessFlagBits::eShaderWrite,
                          vk::ImageLayout::eGeneral,
                          vk::AccessFlagBits::eShaderRead);
        velocity.CopyFrom(commandBuffer, mVelocity);
    });
}

void Extrapolation::Extrapolate()
{
    mExtrapolateCmd.Submit();
}

void Extrapolation::ConstrainVelocity()
{
    mConstrainCmd.Submit();
}

}}
