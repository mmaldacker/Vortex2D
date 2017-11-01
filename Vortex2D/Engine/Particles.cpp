//
//  Particles.cpp
//  Vortex
//

#include "Particles.h"

#include <Vortex2D/Engine/LevelSet.h>

#include <random>

namespace Vortex2D { namespace Fluid {

ParticleCount::ParticleCount(const Renderer::Device& device,
                     const glm::ivec2& size,
                     Renderer::GenericBuffer& particles,
                     const Renderer::DispatchParams& params)
    : Renderer::RenderTexture(device, size.x, size.y, vk::Format::eR32Sint)
    , mDevice(device)
    , mParticles(particles)
    , mNewParticles(device, 8*size.x*size.y)
    , mCount(device, size.x*size.y)
    , mIndex(device, size.x*size.y)
    , mSeeds(device, 4)
    , mDispatchParams(device)
    , mNewDispatchParams(device)
    , mParticleCountWork(device, Renderer::ComputeSize::Default1D(), "../Vortex2D/ParticleCount.comp.spv")
    , mParticleCountBound(mParticleCountWork.Bind({particles, mDispatchParams, *this}))
    , mPrefixScan(device, size)
    , mPrefixScanBound(mPrefixScan.Bind(mCount, mIndex, mNewDispatchParams))
    , mParticleBucketWork(device, Renderer::ComputeSize::Default1D(), "../Vortex2D/ParticleBucket.comp.spv")
    , mParticleBucketBound(mParticleBucketWork.Bind(size, {particles,
                                                           mNewParticles,
                                                           mIndex,
                                                           mCount,
                                                           mDispatchParams}))
    , mParticleSpawnWork(device, size, "../Vortex2D/ParticleSpawn.comp.spv")
    , mParticleSpawnBound(mParticleSpawnWork.Bind({mNewParticles, mIndex, mCount, mSeeds}))
    , mParticlePhiWork(device, size, "../Vortex2D/ParticlePhi.comp.spv")
    , mParticleToGridWork(device, size, "../Vortex2D/ParticleToGrid.comp.spv")
    , mParticleFromGridWork(device, Renderer::ComputeSize::Default1D(), "../Vortex2D/ParticleFromGrid.comp.spv")
    , mCountWork(device, false)
    , mScanWork(device, false)
    , mDispatchCountWork(device)
    , mParticlePhi(device, false)
    , mParticleToGrid(device, false)
    , mParticleFromGrid(device, false)
{
    Renderer::Buffer<Renderer::DispatchParams> localDispatchParams(device, 1, true);
    Renderer::CopyFrom(localDispatchParams, params);
    Renderer::ExecuteCommand(device, [&](vk::CommandBuffer commandBuffer)
    {
        mDispatchParams.CopyFrom(commandBuffer, localDispatchParams);
    });

    // TODO should limit to 4 (or 8) particles
    mCountWork.Record([&](vk::CommandBuffer commandBuffer)
    {
        Clear(commandBuffer, std::array<int, 4>{0, 0, 0, 0});
        mParticleCountBound.RecordIndirect(commandBuffer, mDispatchParams);
        Barrier(commandBuffer,
                vk::ImageLayout::eGeneral, vk::AccessFlagBits::eShaderWrite,
                vk::ImageLayout::eGeneral, vk::AccessFlagBits::eShaderRead);
    });

    mScanWork.Record([&](vk::CommandBuffer commandBuffer)
    {
        mCount.CopyFrom(commandBuffer, *this);
        mPrefixScanBound.Record(commandBuffer);
        mIndex.Barrier(commandBuffer, vk::AccessFlagBits::eShaderWrite, vk::AccessFlagBits::eShaderRead);
        mParticleBucketBound.RecordIndirect(commandBuffer, mDispatchParams);
        mNewParticles.Barrier(commandBuffer, vk::AccessFlagBits::eShaderWrite, vk::AccessFlagBits::eShaderRead);
        mSeeds.Upload(commandBuffer);
        mParticleSpawnBound.Record(commandBuffer);
        mNewParticles.Barrier(commandBuffer, vk::AccessFlagBits::eShaderWrite, vk::AccessFlagBits::eShaderRead);
        particles.CopyFrom(commandBuffer, mNewParticles);
        mDispatchParams.CopyFrom(commandBuffer, mNewDispatchParams);
    });

    mDispatchCountWork.Record([&](vk::CommandBuffer commandBuffer)
    {
        mDispatchParams.Download(commandBuffer);
    });
}

void ParticleCount::Count()
{
    mCountWork.Submit();
}

void ParticleCount::Scan()
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis;

    std::vector<glm::ivec2> seeds = {{dis(gen), dis(gen)},
                                     {dis(gen), dis(gen)},
                                     {dis(gen), dis(gen)},
                                     {dis(gen), dis(gen)}};

    Renderer::CopyFrom(mSeeds, seeds);
    mScanWork.Submit();
}

int ParticleCount::GetCount()
{
    mDispatchCountWork.Submit();
    mDispatchCountWork.Wait();

    Renderer::DispatchParams params(0);
    Renderer::CopyTo(mDispatchParams, params);
    return params.count;
}

Renderer::GenericBuffer& ParticleCount::GetDispatchParams()
{
    return mDispatchParams;
}

void ParticleCount::InitLevelSet(LevelSet& levelSet)
{
    // TODO should shrink wrap wholes and redistance
    mParticlePhiBound = mParticlePhiWork.Bind({*this, mParticles, mIndex, levelSet});
    mParticlePhi.Record([&](vk::CommandBuffer commandBuffer)
    {
        levelSet.Clear(commandBuffer, std::array<float, 4>{3.0f, 0.0f, 0.0f, 0.0f});
        mParticlePhiBound.Record(commandBuffer);
        levelSet.Barrier(commandBuffer,
                         vk::ImageLayout::eGeneral, vk::AccessFlagBits::eShaderWrite,
                         vk::ImageLayout::eGeneral, vk::AccessFlagBits::eShaderRead);
    });
}

void ParticleCount::Phi()
{
    mParticlePhi.Submit();
}

void ParticleCount::InitVelocities(Renderer::Texture& velocity, Renderer::GenericBuffer& valid)
{
    mParticleToGridBound = mParticleToGridWork.Bind({*this, mParticles, mIndex, velocity, valid});
    mParticleToGrid.Record([&](vk::CommandBuffer commandBuffer)
    {
        valid.Clear(commandBuffer);
        mParticleToGridBound.Record(commandBuffer);
        velocity.Barrier(commandBuffer,
                         vk::ImageLayout::eGeneral, vk::AccessFlagBits::eShaderWrite,
                         vk::ImageLayout::eGeneral, vk::AccessFlagBits::eShaderRead);
    });

    mParticleFromGridBound = mParticleFromGridWork.Bind({mParticles, mDispatchParams, velocity});
    mParticleFromGrid.Record([&](vk::CommandBuffer commandBuffer)
    {
       mParticleFromGridBound.RecordIndirect(commandBuffer, mDispatchParams);
       mParticles.Barrier(commandBuffer, vk::AccessFlagBits::eShaderWrite, vk::AccessFlagBits::eShaderRead);
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

}}
