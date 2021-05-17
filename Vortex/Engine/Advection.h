//
//  Advection.h
//  Vortex
//

#pragma once

#include <Vortex/Renderer/Buffer.h>
#include <Vortex/Renderer/CommandBuffer.h>
#include <Vortex/Renderer/Work.h>

#include <Vortex/Engine/Velocity.h>

namespace Vortex
{
namespace Fluid
{
class Density;

/**
 * @brief Advects particles, velocity field or any field using a velocity field.
 */
class Advection
{
public:
  /**
   * @brief Initialize advection kernels and related object.
   * @param device vulkan device
   * @param size size of velocity field
   * @param dt delta time for integration
   * @param velocity velocity field
   */
  VORTEX_API Advection(const Renderer::Device& device,
                       const glm::ivec2& size,
                       float dt,
                       Velocity& velocity,
                       Velocity::InterpolationMode interpolationMode);

  /**
   * @brief Self advect velocity
   */
  VORTEX_API void AdvectVelocity();

  // TODO can only advect one field, need to be able to do as many as we want
  /**
   * @brief Binds a density field to be advected.
   * @param density density field
   */
  VORTEX_API void AdvectBind(Density& density);

  /**
   * @brief Performs an advection of the density field. Asynchronous operation.
   */
  VORTEX_API void Advect();

  /**
   * @brief Binds praticles to be advected.
   * Also use a level set to project out the particles if they enter it.
   * @param particles particles to be advected
   * @param levelSet level set to project out particles
   * @param dispatchParams contains number of particles
   */
  VORTEX_API void AdvectParticleBind(
      Renderer::GenericBuffer& particles,
      Renderer::Texture& levelSet,
      Renderer::IndirectBuffer<Renderer::DispatchParams>& dispatchParams);
  /**
   * @brief Advect particles. Asynchrounous operation.
   */
  VORTEX_API void AdvectParticles();

private:
  const Renderer::Device& mDevice;
  float mDt;
  glm::ivec2 mSize;
  Velocity& mVelocity;

  Renderer::Work mVelocityAdvect;
  Renderer::Work::Bound mVelocityAdvectBound;
  Renderer::Work mAdvect;
  Renderer::Work::Bound mAdvectBound;
  Renderer::Work mAdvectParticles;
  Renderer::Work::Bound mAdvectParticlesBound;

  Renderer::CommandBuffer mAdvectVelocityCmd;
  Renderer::CommandBuffer mAdvectCmd;
  Renderer::CommandBuffer mAdvectParticlesCmd;
};

}  // namespace Fluid
}  // namespace Vortex
