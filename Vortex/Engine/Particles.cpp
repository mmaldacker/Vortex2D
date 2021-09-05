//
//  Particles.cpp
//  Vortex
//

#include "Particles.h"

#include <Vortex/Engine/LevelSet.h>

#include <random>
#include "vortex_generated_spirv.h"

namespace Vortex
{
namespace Fluid
{
float DefaultParticleSize()
{
  return 1.0f / std::sqrt(2.0f);
}

ParticleCount::ParticleCount(const Renderer::Device& device,
                             const glm::ivec2& size,
                             Renderer::GenericBuffer& particles,
                             Velocity::InterpolationMode interpolationMode,
                             const Renderer::DispatchParams& params,
                             float alpha,
                             float particleSize)
    : Renderer::RenderTexture(device, size.x, size.y, vk::Format::eR32Sint)
    , mDevice(device)
    , mSize(size)
    , mParticles(particles)
    , mNewParticles(device, 8 * size.x * size.y)
    , mDelta(device, size.x * size.y)
    , mCount(device, size.x * size.y)
    , mIndex(device, size.x * size.y)
    , mSeeds(device, 4, VMA_MEMORY_USAGE_CPU_TO_GPU)
    , mDispatchParams(device)
    , mLocalDispatchParams(device, 1, VMA_MEMORY_USAGE_CPU_ONLY)
    , mNewDispatchParams(device)
    , mParticleCountWork(device, Renderer::ComputeSize::Default1D(), SPIRV::ParticleCount_comp)
    , mParticleCountBound(mParticleCountWork.Bind(size, {particles, mDispatchParams, mDelta}))
    , mParticleClampWork(device, size, SPIRV::ParticleClamp_comp)
    , mParticleClampBound(mParticleClampWork.Bind(size, {mDelta}))
    , mPrefixScan(device, size.x * size.y)
    , mPrefixScanBound(mPrefixScan.Bind(mDelta, mIndex, mNewDispatchParams))
    , mParticleBucketWork(device, Renderer::ComputeSize::Default1D(), SPIRV::ParticleBucket_comp)
    , mParticleBucketBound(
          mParticleBucketWork.Bind(size,
                                   {particles, mNewParticles, mIndex, mDelta, mDispatchParams}))
    , mParticleSpawnWork(device, size, SPIRV::ParticleSpawn_comp)
    , mParticleSpawnBound(mParticleSpawnWork.Bind({mNewParticles, mIndex, mDelta, mSeeds}))
    , mParticlePhiWork(device,
                       size,
                       SPIRV::ParticlePhi_comp,
                       Renderer::SpecConst(Renderer::SpecConstValue(3, particleSize)))
    , mParticleToGridWork(device, size, SPIRV::ParticleToGrid_comp)
    , mParticleFromGridWork(device,
                            Renderer::ComputeSize::Default1D(),
                            SPIRV::ParticleFromGrid_comp,
                            Renderer::SpecConst(Renderer::SpecConstValue(3, interpolationMode)))
    , mScanWork(device, false)
    , mDispatchCountWork(device)
    , mParticlePhi(device, false)
    , mParticleToGrid(device, false)
    , mParticleFromGrid(device, false)
    , mAlpha(alpha)
{
  Renderer::CopyFrom(mLocalDispatchParams, params);
  device.Execute([&](vk::CommandBuffer commandBuffer)
                 { mDispatchParams.CopyFrom(commandBuffer, mLocalDispatchParams); });

  // TODO clamp should be configurable

  // Algorithm
  // 1) copy this to mDelta
  //    -> this sets the number of particles we want to add or remove in each
  //    grid cell
  // 2) for each particle, increase count in grid cell mDelta
  // 3) clamp grid cell of mDelta count between [0, 8]
  //    -> now mDelta contains the number of particles we want in each cell.
  //       which means deleting some or add some
  // 4) copy mDelta to mCount
  //    -> we save the count of particles in mCount as we'll modify mDelta
  // 5) prefix scan from mDelta to mIndex
  //    -> mIndex now maps from grid cell to particle index
  // 6) for each particle, if count in grid cell mDelta > 0, copy to new
  // particles and decrease count
  //    -> using the mIndex mapping to get the index in the new particles buffer
  // 7) for each grid cell mDelta > 0, add new particle in new particles
  //    -> set the new particles with random position
  // 8) copy new particles to particles

  mScanWork.Record(
      [&](vk::CommandBuffer commandBuffer)
      {
        commandBuffer.debugMarkerBeginEXT({"Particle count", {{0.14f, 0.39f, 0.12f, 1.0f}}},
                                          mDevice.Loader());
        mDelta.CopyFrom(commandBuffer, *this);
        Clear(commandBuffer, std::array<int, 4>{0, 0, 0, 0});
        mParticleCountBound.RecordIndirect(commandBuffer, mDispatchParams);
        mDelta.Barrier(
            commandBuffer, vk::AccessFlagBits::eShaderWrite, vk::AccessFlagBits::eShaderRead);
        mParticleClampBound.Record(commandBuffer);
        mDelta.Barrier(
            commandBuffer, vk::AccessFlagBits::eShaderWrite, vk::AccessFlagBits::eShaderRead);
        mCount.CopyFrom(commandBuffer, mDelta);
        commandBuffer.debugMarkerEndEXT(mDevice.Loader());

        commandBuffer.debugMarkerBeginEXT({"Particle scan", {{0.59f, 0.20f, 0.35f, 1.0f}}},
                                          mDevice.Loader());
        mPrefixScanBound.Record(commandBuffer);
        mParticleBucketBound.RecordIndirect(commandBuffer, mDispatchParams);
        mNewParticles.Barrier(
            commandBuffer, vk::AccessFlagBits::eShaderWrite, vk::AccessFlagBits::eShaderRead);
        mParticleSpawnBound.Record(commandBuffer);
        mNewParticles.Barrier(
            commandBuffer, vk::AccessFlagBits::eShaderWrite, vk::AccessFlagBits::eShaderRead);
        particles.CopyFrom(commandBuffer, mNewParticles);
        mDispatchParams.CopyFrom(commandBuffer, mNewDispatchParams);
        commandBuffer.debugMarkerEndEXT(mDevice.Loader());
      });

  mDispatchCountWork.Record([&](vk::CommandBuffer commandBuffer)
                            { mLocalDispatchParams.CopyFrom(commandBuffer, mDispatchParams); });
}

void ParticleCount::Scan()
{
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<> dis;

  std::vector<glm::ivec2> seeds = {
      {dis(gen), dis(gen)}, {dis(gen), dis(gen)}, {dis(gen), dis(gen)}, {dis(gen), dis(gen)}};

  Renderer::CopyFrom(mSeeds, seeds);
  mScanWork.Submit();
}

int ParticleCount::GetTotalCount()
{
  mDispatchCountWork.Submit().Wait();

  Renderer::DispatchParams params(0);
  Renderer::CopyTo(mLocalDispatchParams, params);
  return params.count;
}

Renderer::IndirectBuffer<Renderer::DispatchParams>& ParticleCount::GetDispatchParams()
{
  return mDispatchParams;
}

void ParticleCount::LevelSetBind(LevelSet& levelSet)
{
  // TODO should shrink wrap wholes and redistance
  mParticlePhiBound = mParticlePhiWork.Bind({mCount, mParticles, mIndex, levelSet});
  mParticlePhi.Record(
      [&](vk::CommandBuffer commandBuffer)
      {
        commandBuffer.debugMarkerBeginEXT({"Particle phi", {{0.86f, 0.72f, 0.29f, 1.0f}}},
                                          mDevice.Loader());
        levelSet.Clear(commandBuffer, std::array<float, 4>{3.0f, 0.0f, 0.0f, 0.0f});
        mParticlePhiBound.Record(commandBuffer);
        levelSet.Barrier(commandBuffer,
                         vk::ImageLayout::eGeneral,
                         vk::AccessFlagBits::eShaderWrite,
                         vk::ImageLayout::eGeneral,
                         vk::AccessFlagBits::eShaderRead);
        commandBuffer.debugMarkerEndEXT(mDevice.Loader());
      });
}

void ParticleCount::Phi()
{
  mParticlePhi.Submit();
}

void ParticleCount::VelocitiesBind(Velocity& velocity, Renderer::GenericBuffer& valid)
{
  mParticleToGridBound = mParticleToGridWork.Bind({mCount, mParticles, mIndex, velocity, valid});
  mParticleToGrid.Record(
      [&](vk::CommandBuffer commandBuffer)
      {
        commandBuffer.debugMarkerBeginEXT({"Particle to grid", {{0.71f, 0.15f, 0.48f, 1.0f}}},
                                          mDevice.Loader());
        valid.Clear(commandBuffer);
        mParticleToGridBound.Record(commandBuffer);
        valid.Barrier(
            commandBuffer, vk::AccessFlagBits::eShaderWrite, vk::AccessFlagBits::eShaderRead);
        commandBuffer.debugMarkerEndEXT(mDevice.Loader());
      });

  mParticleFromGridBound =
      mParticleFromGridWork.Bind({mParticles, mDispatchParams, velocity, velocity.D()});
  mParticleFromGrid.Record(
      [&](vk::CommandBuffer commandBuffer)
      {
        commandBuffer.debugMarkerBeginEXT({"Particle from grid", {{0.35f, 0.11f, 0.87f, 1.0f}}},
                                          mDevice.Loader());
        mParticleFromGridBound.PushConstant(commandBuffer, mSize.x, mSize.y, mAlpha);
        mParticleFromGridBound.RecordIndirect(commandBuffer, mDispatchParams);
        mParticles.Barrier(
            commandBuffer, vk::AccessFlagBits::eShaderWrite, vk::AccessFlagBits::eShaderRead);
        commandBuffer.debugMarkerEndEXT(mDevice.Loader());
      });
}

void ParticleCount::TransferToGrid()
{
  mParticleToGrid.Submit();
}

void ParticleCount::TransferFromGrid()
{
  mParticleFromGrid.Submit();
}

}  // namespace Fluid
}  // namespace Vortex
