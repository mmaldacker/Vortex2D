//
//  Advection.cpp
//  Vortex
//

#include "Advection.h"

#include <Vortex2D/Renderer/Pipeline.h>
#include <Vortex2D/Engine/Density.h>

namespace Vortex2D { namespace Fluid {


Advection::Advection(const Renderer::Device& device, const glm::ivec2& size, float dt, Renderer::Texture& velocity)
    : mDt(dt)
    , mSize(size)
    , mVelocity(velocity)
    , mTmpVelocity(device, size.x, size.y, vk::Format::eR32G32Sfloat)
    , mVelocityAdvect(device, size, "../Vortex2D/AdvectVelocity.comp.spv")
    , mVelocityAdvectBound(mVelocityAdvect.Bind({velocity, mTmpVelocity}))
    , mAdvect(device, size, "../Vortex2D/Advect.comp.spv")
    , mAdvectParticles(device, Renderer::ComputeSize::Default1D(), "../Vortex2D/AdvectParticles.comp.spv")
    , mAdvectVelocityCmd(device, false)
    , mAdvectCmd(device, false)
    , mAdvectParticlesCmd(device, false)
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

void Advection::AdvectInit(Density& density)
{
    mAdvectBound = mAdvect.Bind({mVelocity, density, density.mFieldBack});
    mAdvectCmd.Record([&](vk::CommandBuffer commandBuffer)
    {
        mAdvectBound.PushConstant(commandBuffer, 8, mDt);
        mAdvectBound.Record(commandBuffer);
        density.mFieldBack.Barrier(commandBuffer,
                      vk::ImageLayout::eGeneral,
                      vk::AccessFlagBits::eShaderWrite,
                      vk::ImageLayout::eGeneral,
                      vk::AccessFlagBits::eShaderRead);
        density.CopyFrom(commandBuffer, density.mFieldBack);
    });
}

void Advection::Advect()
{
    mAdvectCmd.Submit();
}

void Advection::AdvectParticleInit(Renderer::GenericBuffer& particles,
                                   Renderer::Texture& levelSet,
                                   Renderer::GenericBuffer& dispatchParams)
{
    mAdvectParticlesBound = mAdvectParticles.Bind(mSize, {particles, dispatchParams, mVelocity, levelSet});
    mAdvectParticlesCmd.Record([&](vk::CommandBuffer commandBuffer)
    {
        commandBuffer.debugMarkerBeginEXT({"Particle advect", {{ 0.09f, 0.17f, 0.36f, 1.0f}}});
        mAdvectParticlesBound.PushConstant(commandBuffer, 8, mDt);
        mAdvectParticlesBound.RecordIndirect(commandBuffer, dispatchParams);
        particles.Barrier(commandBuffer, vk::AccessFlagBits::eShaderWrite, vk::AccessFlagBits::eShaderRead);
        commandBuffer.debugMarkerEndEXT();
    });
}

void Advection::AdvectParticles()
{
    mAdvectParticlesCmd.Submit();
}

}}
