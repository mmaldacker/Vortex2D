//
//  Particles.cpp
//  Vortex
//

#include "Particles.h"

namespace Vortex2D { namespace Fluid {

Particles::Particles(const Renderer::Device& device,
                     const glm::ivec2& size,
                     Renderer::Buffer& particles,
                     const Renderer::DispatchParams& params)
    : Renderer::RenderTexture(device, size.x, size.y, vk::Format::eR32Sint)
    , mNewParticles(device, vk::BufferUsageFlagBits::eStorageBuffer, false, 8*size.x*size.y*sizeof(Particle))
    , mCount(device, vk::BufferUsageFlagBits::eStorageBuffer, false, size.x*size.y*sizeof(int))
    , mIndex(device, vk::BufferUsageFlagBits::eStorageBuffer, false, size.x*size.y*sizeof(int))
    , mDispatchParams(device, vk::BufferUsageFlagBits::eStorageBuffer, false, sizeof(params))
    , mNewDispatchParams(device, vk::BufferUsageFlagBits::eStorageBuffer, false, sizeof(params))
    , mParticleCountWork(device, Renderer::ComputeSize::Default1D(), "../Vortex2D/ParticleCount.comp.spv",
                         {vk::DescriptorType::eStorageBuffer,
                          vk::DescriptorType::eStorageBuffer,
                          vk::DescriptorType::eStorageImage})
    , mParticleCountBound(mParticleCountWork.Bind({particles, mDispatchParams, *this}))
    , mPrefixScan(device, size)
    , mPrefixScanBound(mPrefixScan.Bind(mCount, mIndex, mNewDispatchParams))
    , mParticleBucketWork(device, Renderer::ComputeSize::Default1D(), "../Vortex2D/ParticleBucket.comp.spv",
                          {vk::DescriptorType::eStorageBuffer,
                           vk::DescriptorType::eStorageBuffer,
                           vk::DescriptorType::eStorageBuffer,
                           vk::DescriptorType::eStorageBuffer,
                           vk::DescriptorType::eStorageBuffer})
    , mParticleBucketBound(mParticleBucketWork.Bind(size, {particles,
                                                           mNewParticles,
                                                           mIndex,
                                                           mCount,
                                                           mDispatchParams}))
    , mCountWork(device, false)
    , mScanWork(device, false)
{
    Renderer::Buffer localDispathParams(device, vk::BufferUsageFlagBits::eStorageBuffer, true, sizeof(params));
    localDispathParams.CopyFrom(params);
    Renderer::ExecuteCommand(device, [&](vk::CommandBuffer commandBuffer)
    {
        mDispatchParams.CopyFrom(commandBuffer, localDispathParams);
    });

    mCountWork.Record([&](vk::CommandBuffer commandBuffer)
    {
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
        // TODO spawn particles
        particles.CopyFrom(commandBuffer, mNewParticles);
        mDispatchParams.CopyFrom(commandBuffer, mNewDispatchParams);
    });
}

void Particles::Count()
{
    mCountWork.Submit();
}

void Particles::Scan()
{
    mScanWork.Submit();
}


}}
