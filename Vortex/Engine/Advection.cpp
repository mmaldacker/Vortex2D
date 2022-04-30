//
//  Advection.cpp
//  Vortex
//

#include "Advection.h"

#include <Vortex/Engine/Density.h>
#include <Vortex/Renderer/Pipeline.h>

#include "vortex_generated_spirv.h"

namespace Vortex
{
namespace Fluid
{
Advection::Advection(Renderer::Device& device,
                     const glm::ivec2& size,
                     float dt,
                     Velocity& velocity,
                     Velocity::InterpolationMode interpolationMode)
    : mDevice(device)
    , mDt(dt)
    , mSize(size)
    , mVelocity(velocity)
    , mVelocityAdvect(device,
                      Renderer::ComputeSize{size},
                      SPIRV::AdvectVelocity_comp,
                      Renderer::SpecConst(Renderer::SpecConstValue(3, interpolationMode)))
    , mVelocityAdvectBound(mVelocityAdvect.Bind({velocity, velocity.Output()}))
    , mAdvect(device, Renderer::ComputeSize{size}, SPIRV::Advect_comp)
    , mAdvectParticles(device,
                       Renderer::ComputeSize::Default1D(),
                       SPIRV::AdvectParticles_comp,
                       Renderer::SpecConst(Renderer::SpecConstValue(3, interpolationMode)))
    , mAdvectVelocityCmd(device, false)
    , mAdvectCmd(device, false)
    , mAdvectParticlesCmd(device, false)
{
  mAdvectVelocityCmd.Record(
      [&](Renderer::CommandEncoder& command)
      {
        command.DebugMarkerBegin("Velocity advect", {0.15f, 0.46f, 0.19f, 1.0f});
        mVelocityAdvectBound.PushConstant(command, dt);
        mVelocityAdvectBound.Record(command);
        velocity.Output().Barrier(command,
                                  Renderer::ImageLayout::General,
                                  Renderer::Access::Write,
                                  Renderer::ImageLayout::General,
                                  Renderer::Access::Read);
        velocity.CopyBack(command);
        command.DebugMarkerEnd();
      });
}

void Advection::AdvectVelocity()
{
  mAdvectVelocityCmd.Submit();
}

void Advection::AdvectBind(Density& density)
{
  mAdvectBound = mAdvect.Bind({mVelocity, density, density.mFieldBack});
  mAdvectCmd.Record(
      [&](Renderer::CommandEncoder& command)
      {
        command.DebugMarkerBegin("Density advect", {0.86f, 0.14f, 0.52f, 1.0f});
        mAdvectBound.PushConstant(command, mDt);
        mAdvectBound.Record(command);
        density.mFieldBack.Barrier(command,
                                   Renderer::ImageLayout::General,
                                   Renderer::Access::Write,
                                   Renderer::ImageLayout::General,
                                   Renderer::Access::Read);
        density.CopyFrom(command, density.mFieldBack);
        command.DebugMarkerEnd();
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
  mAdvectParticlesBound = mAdvectParticles.Bind(Renderer::ComputeSize{mSize},
                                                {particles, dispatchParams, mVelocity, levelSet});
  mAdvectParticlesCmd.Record(
      [&](Renderer::CommandEncoder& command)
      {
        command.DebugMarkerBegin("Particle advect", {0.09f, 0.17f, 0.36f, 1.0f});
        mAdvectParticlesBound.PushConstant(command, mDt);
        mAdvectParticlesBound.RecordIndirect(command, dispatchParams);
        particles.Barrier(command, Renderer::Access::Write, Renderer::Access::Read);
        command.DebugMarkerEnd();
      });
}

void Advection::AdvectParticles()
{
  mAdvectParticlesCmd.Submit();
}

}  // namespace Fluid
}  // namespace Vortex
