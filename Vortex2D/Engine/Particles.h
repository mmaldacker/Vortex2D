//
//  Particles.h
//  Vortex
//

#ifndef Vortex2d_Particles_h
#define Vortex2d_Particles_h

#include <Vortex2D/Renderer/Buffer.h>
#include <Vortex2D/Renderer/RenderTexture.h>
#include <Vortex2D/Engine/PrefixScan.h>
#include <Vortex2D/Engine/Velocity.h>

namespace Vortex2D { namespace Fluid {

class LevelSet;

struct Particle
{
  alignas(8) glm::vec2 Position;
  alignas(8) glm::vec2 Velocity;
};

/**
 * @brief Container for particles used in the advection of the fluid simulation.
 * Also a level set that is built from the particles.
 */
class ParticleCount : public Renderer::RenderTexture
{
public:
    VORTEX2D_API ParticleCount(const Renderer::Device& device,
                               const glm::ivec2& size,
                               Renderer::GenericBuffer& particles,
                               const Renderer::DispatchParams& params = {0},
                               float alpha = 1.0f);

    /**
     * @brief Count the number of particles and update the internal data structures.
     */
    VORTEX2D_API void Scan();

    /**
     * @brief Calculate the total number of particles and return it.
     * @return
     */
    VORTEX2D_API int GetTotalCount();

    /**
     * @brief Calculate the dispatch parameters to use on the particle buffer
     * @return
     */
    VORTEX2D_API Renderer::IndirectBuffer<Renderer::DispatchParams>& GetDispatchParams();

    /**
     * @brief Bind a solid level set, which will be used to interpolate the particles out of.
     * @param levelSet
     */
    VORTEX2D_API void LevelSetBind(LevelSet& levelSet);

    /**
     * @brief Calculate the level set from the particles.
     */
    VORTEX2D_API void Phi();

    /**
     * @brief Bind the velocities, used for advection of the particles.
     * @param velocity
     * @param valid
     */
    VORTEX2D_API void VelocitiesBind(Velocity& velocity, Renderer::GenericBuffer& valid);

    /**
     * @brief Interpolate the velocities of the particles to the velocities field.
     */
    VORTEX2D_API void TransferToGrid();

    /**
     * @brief Interpolate the velocities field in to the particles' velocity.
     */
    VORTEX2D_API void TransferFromGrid();

private:
    const Renderer::Device& mDevice;
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

}}

#endif
