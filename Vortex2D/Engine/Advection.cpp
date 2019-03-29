//
//  Advection.cpp
//  Vortex
//

#include "Advection.h"

#include <Vortex2D/Engine/Density.h>
#include <Vortex2D/Renderer/Pipeline.h>

#include "vortex2d_generated_spirv.h"

namespace Vortex2D
{
namespace Fluid
{
Advection::Advection(const Renderer::Device& device,
                     const glm::ivec2& size,
                     float dt,
                     Velocity& velocity)
    : mDt(dt)
    , mSize(size)
    , mVelocity(velocity)
    , mVelocityAdvect(device, size, SPIRV::AdvectVelocity_comp)
    , mVelocityAdvectBound(mVelocityAdvect.Bind({velocity, velocity.Output()}))
    , mAdvect(device, size, SPIRV::Advect_comp)
    , mAdvectParticles(device, Renderer::ComputeSize::Default1D(), SPIRV::AdvectParticles_comp)
    , mAdvectVelocityCmd(device, false)
    , mAdvectCmd(device, false)
    , mAdvectParticlesCmd(device, false)
{
  mAdvectVelocityCmd.Record([&](vk::CommandBuffer commandBuffer) {
    commandBuffer.debugMarkerBeginEXT({"Velocity advect", {{0.15f, 0.46f, 0.19f, 1.0f}}});
    mVelocityAdvectBound.PushConstant(commandBuffer, dt);
    mVelocityAdvectBound.Record(commandBuffer);
    velocity.CopyBack(commandBuffer);
    commandBuffer.debugMarkerEndEXT();
  });
}

void Advection::AdvectVelocity()
{
  mAdvectVelocityCmd.Submit();
}

void Advection::AdvectBind(Density& density)
{
  mAdvectBound = mAdvect.Bind({mVelocity, density, density.mFieldBack});
  mAdvectCmd.Record([&](vk::CommandBuffer commandBuffer) {
    commandBuffer.debugMarkerBeginEXT({"Density advect", {{0.86f, 0.14f, 0.52f, 1.0f}}});
    mAdvectBound.PushConstant(commandBuffer, mDt);
    mAdvectBound.Record(commandBuffer);
    density.mFieldBack.Barrier(commandBuffer,
                               vk::ImageLayout::eGeneral,
                               vk::AccessFlagBits::eShaderWrite,
                               vk::ImageLayout::eGeneral,
                               vk::AccessFlagBits::eShaderRead);
    density.CopyFrom(commandBuffer, density.mFieldBack);
    commandBuffer.debugMarkerEndEXT();
  });
}

void Advection::Advect()
{
  if (mAdvectCmd)
  {
    mAdvectCmd.Submit();
  }
}

void Advection::AdvectParticleBind(
    Renderer::GenericBuffer& particles,
    Renderer::Texture& levelSet,
    Renderer::IndirectBuffer<Renderer::DispatchParams>& dispatchParams)
{
  mAdvectParticlesBound =
      mAdvectParticles.Bind(mSize, {particles, dispatchParams, mVelocity, levelSet});
  mAdvectParticlesCmd.Record([&](vk::CommandBuffer commandBuffer) {
    commandBuffer.debugMarkerBeginEXT({"Particle advect", {{0.09f, 0.17f, 0.36f, 1.0f}}});
    mAdvectParticlesBound.PushConstant(commandBuffer, mDt);
    mAdvectParticlesBound.RecordIndirect(commandBuffer, dispatchParams);
    particles.Barrier(
        commandBuffer, vk::AccessFlagBits::eShaderWrite, vk::AccessFlagBits::eShaderRead);
    commandBuffer.debugMarkerEndEXT();
  });
}

void Advection::AdvectParticles()
{
  mAdvectParticlesCmd.Submit();
}

}  // namespace Fluid
}  // namespace Vortex2D
