//
//  Particles.h
//  Vortex
//

#pragma once

#include <Vortex/Engine/PrefixScan.h>
#include <Vortex/Engine/Velocity.h>
#include <Vortex/Renderer/Buffer.h>
#include <Vortex/Renderer/RenderTexture.h>

namespace Vortex
{
namespace Fluid
{
class LevelSet;

struct Particle
{
  alignas(8) glm::vec2 Position;
  alignas(8) glm::vec2 Velocity;
};

VORTEX_API float DefaultParticleSize();

/**
 * @brief Container for particles used in the advection of the fluid simulation.
 * Also a level set that is built from the particles.
 */
class ParticleCount : public Renderer::RenderTexture
{
public:
  VORTEX_API ParticleCount(Renderer::Device& device,
                           const glm::ivec2& size,
                           Renderer::GenericBuffer& particles,
                           Velocity::InterpolationMode interpolationMode,
                           const Renderer::DispatchParams& params = {0},
                           float alpha = 1.0f,
                           float particleSize = DefaultParticleSize());

  /**
   * @brief Count the number of particles and update the internal data
   * structures.
   */
  VORTEX_API void Scan();

  /**
   * @brief Calculate the total number of particles and return it.
   * @return
   */
  VORTEX_API int GetTotalCount();

  /**
   * @brief Calculate the dispatch parameters to use on the particle buffer
   * @return
   */
  VORTEX_API Renderer::IndirectBuffer<Renderer::DispatchParams>& GetDispatchParams();

  /**
   * @brief Bind a solid level set, which will be used to interpolate the
   * particles out of.
   * @param levelSet
   */
  VORTEX_API void LevelSetBind(LevelSet& levelSet);

  /**
   * @brief Calculate the level set from the particles.
   */
  VORTEX_API void Phi();

  /**
   * @brief Bind the velocities, used for advection of the particles.
   * @param velocity
   * @param valid
   */
  VORTEX_API void VelocitiesBind(Velocity& velocity, Renderer::GenericBuffer& valid);

  /**
   * @brief Interpolate the velocities of the particles to the velocities field.
   */
  VORTEX_API void TransferToGrid();

  /**
   * @brief Interpolate the velocities field in to the particles' velocity.
   */
  VORTEX_API void TransferFromGrid();

private:
  Renderer::Device& mDevice;
  glm::ivec2 mSize;
  Renderer::GenericBuffer& mParticles;
  Renderer::Buffer<Particle> mNewParticles;
  Renderer::Buffer<int> mDelta, mCount;
  Renderer::Buffer<int> mIndex;
  Renderer::Buffer<glm::ivec2> mSeeds;

  Renderer::IndirectBuffer<Renderer::DispatchParams> mDispatchParams;
  Renderer::Buffer<Renderer::DispatchParams> mLocalDispatchParams, mNewDispatchParams;

  Renderer::Work mParticleCountWork;
  Renderer::Work::Bound mParticleCountBound;
  Renderer::Work mParticleClampWork;
  Renderer::Work::Bound mParticleClampBound;
  PrefixScan mPrefixScan;
  PrefixScan::Bound mPrefixScanBound;
  Renderer::Work mParticleBucketWork;
  Renderer::Work::Bound mParticleBucketBound;
  Renderer::Work mParticleSpawnWork;
  Renderer::Work::Bound mParticleSpawnBound;
  Renderer::Work mParticlePhiWork;
  Renderer::Work::Bound mParticlePhiBound;
  Renderer::Work mParticleToGridWork;
  Renderer::Work::Bound mParticleToGridBound;
  Renderer::Work mParticleFromGridWork;
  Renderer::Work::Bound mParticleFromGridBound;

  Renderer::CommandBuffer mScanWork;
  Renderer::CommandBuffer mDispatchCountWork;
  Renderer::CommandBuffer mParticlePhi;
  Renderer::CommandBuffer mParticleToGrid;
  Renderer::CommandBuffer mParticleFromGrid;

  float mAlpha;
};

}  // namespace Fluid
}  // namespace Vortex
