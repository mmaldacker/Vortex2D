//
//  Extrapolation.cpp
//  Vortex2D

#include "Extrapolation.h"

namespace Vortex2D { namespace Fluid {

Extrapolation::Extrapolation(const Renderer::Device& device,
                             const glm::ivec2& size,
                             Renderer::GenericBuffer& valid,
                             Renderer::Texture& velocity,
                             int iterations)
    : mValid(device,size.x*size.y)
    , mVelocity(velocity)
    , mTempVelocity(device, size.x, size.y, vk::Format::eR32G32Sfloat)
    , mExtrapolateVelocity(device, size, "../Vortex2D/ExtrapolateVelocity.comp.spv")
    , mExtrapolateVelocityFrontBound(mExtrapolateVelocity.Bind({valid, mValid, velocity}))
    , mExtrapolateVelocityBackBound(mExtrapolateVelocity.Bind({mValid, valid, velocity}))
    , mConstrainVelocity(device, size, "../Vortex2D/ConstrainVelocity.comp.spv")
    , mExtrapolateCmd(device, false)
    , mConstrainCmd(device, false)
{
    mExtrapolateCmd.Record([&, iterations](vk::CommandBuffer commandBuffer)
    {
        commandBuffer.debugMarkerBeginEXT({"Extrapolate", {{ 0.60f, 0.87f, 0.12f, 1.0f}}});
        for (int i = 0; i < iterations / 2; i++)
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
        commandBuffer.debugMarkerEndEXT();
    });
}

void Extrapolation::Extrapolate()
{
    mExtrapolateCmd.Submit();
}

void Extrapolation::ConstrainInit(Renderer::Texture& solidPhi)
{
    mConstrainVelocityBound = mConstrainVelocity.Bind({solidPhi, mVelocity, mTempVelocity});

    mConstrainCmd.Record([&](vk::CommandBuffer commandBuffer)
    {
        commandBuffer.debugMarkerBeginEXT({"Constrain Velocity", {{ 0.82f, 0.20f, 0.20f, 1.0f}}});
        mConstrainVelocityBound.Record(commandBuffer);
        mTempVelocity.Barrier(commandBuffer,
                          vk::ImageLayout::eGeneral,
                          vk::AccessFlagBits::eShaderWrite,
                          vk::ImageLayout::eGeneral,
                          vk::AccessFlagBits::eShaderRead);
        mVelocity.CopyFrom(commandBuffer, mTempVelocity);
        commandBuffer.debugMarkerEndEXT();
    });
}

void Extrapolation::ConstrainVelocity()
{
    mConstrainCmd.Submit();
}

}}
