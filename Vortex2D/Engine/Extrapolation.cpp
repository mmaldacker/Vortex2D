//
//  Extrapolation.cpp
//  Vortex2D

#include "Extrapolation.h"

#include "vortex2d_generated_spirv.h"

namespace Vortex2D { namespace Fluid {

Extrapolation::Extrapolation(const Renderer::Device& device,
                             const glm::ivec2& size,
                             Renderer::GenericBuffer& valid,
                             Velocity& velocity,
                             int iterations)
    : mValid(device,size.x*size.y)
    , mVelocity(velocity)
    , mExtrapolateVelocity(device, size, ExtrapolateVelocity_comp)
    , mExtrapolateVelocityBound(mExtrapolateVelocity.Bind({valid, mValid, velocity.Input(), velocity.Output()}))
    , mConstrainVelocity(device, size, ConstrainVelocity_comp)
    , mExtrapolateCmd(device, false)
    , mConstrainCmd(device, false)
{
    mExtrapolateCmd.Record([&, iterations](vk::CommandBuffer commandBuffer)
    {
        commandBuffer.debugMarkerBeginEXT({"Extrapolate", {{ 0.60f, 0.87f, 0.12f, 1.0f}}});
        for (int i = 0; i < iterations; i++)
        {
            mExtrapolateVelocityBound.Record(commandBuffer);
            velocity.CopyBack(commandBuffer);
            mValid.Barrier(commandBuffer,
                           vk::AccessFlagBits::eShaderWrite,
                           vk::AccessFlagBits::eShaderRead);
            valid.CopyFrom(commandBuffer, mValid);
        }
        commandBuffer.debugMarkerEndEXT();
    });
}

void Extrapolation::Extrapolate()
{
    mExtrapolateCmd.Submit();
}

void Extrapolation::ConstrainInit(Velocity& solidVelocity, Renderer::Texture& solidPhi)
{
    mConstrainVelocityBound = mConstrainVelocity.Bind({solidPhi, mVelocity.Input(), solidVelocity.Input(), mVelocity.Output()});

    mConstrainCmd.Record([&](vk::CommandBuffer commandBuffer)
    {
        commandBuffer.debugMarkerBeginEXT({"Constrain Velocity", {{ 0.82f, 0.20f, 0.20f, 1.0f}}});
        mConstrainVelocityBound.Record(commandBuffer);
        mVelocity.CopyBack(commandBuffer);
        commandBuffer.debugMarkerEndEXT();
    });
}

void Extrapolation::ConstrainVelocity()
{
    mConstrainCmd.Submit();
}

}}
