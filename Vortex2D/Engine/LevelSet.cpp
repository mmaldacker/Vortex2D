//
//  LevelSet.cpp
//  Vortex2D
//

#include "LevelSet.h"

#include <Vortex2D/Renderer/CommandBuffer.h>

namespace Vortex2D { namespace  Fluid {

LevelSet::LevelSet(const Renderer::Device& device, const glm::vec2& size, int reinitializeIterations)
    : Renderer::RenderTexture(device, size.x, size.y, vk::Format::eR32Sfloat)
    , mLevelSet0(device, size.x, size.y, vk::Format::eR32Sfloat, false)
    , mLevelSetBack(device, size.x, size.y, vk::Format::eR32Sfloat, false)
    , mSampler(Renderer::SamplerBuilder()
               .AddressMode(vk::SamplerAddressMode::eClampToEdge)
               .Create(device.Handle()))
    , mExtrapolate(device, size, "../Vortex2D/Extrapolate.comp.spv",
                   {vk::DescriptorType::eStorageImage,
                    vk::DescriptorType::eStorageImage})
    , mRedistance(device, size, "../Vortex2D/Redistance.comp.spv",
                  {vk::DescriptorType::eCombinedImageSampler,
                   vk::DescriptorType::eCombinedImageSampler,
                   vk::DescriptorType::eStorageImage},
                  4)
    , mRedistanceFront(mRedistance.Bind({{*mSampler, mLevelSet0}, {*mSampler, *this}, mLevelSetBack}))
    , mRedistanceBack(mRedistance.Bind({{*mSampler, mLevelSet0}, {*mSampler, mLevelSetBack}, *this}))
    , mExtrapolateCmd(device)
    , mReinitialiseCmd(device)
    , mRedistanceCmd(device)
{
    const float delta = 0.1f;

    mRedistanceCmd.Record([&](vk::CommandBuffer commandBuffer)
    {
        mLevelSet0.CopyFrom(commandBuffer, *this);

        mRedistanceFront.PushConstant(commandBuffer, 8, delta);
        mRedistanceFront.Record(commandBuffer);
        mLevelSetBack.Barrier(commandBuffer,
                              vk::ImageLayout::eGeneral,
                              vk::AccessFlagBits::eShaderWrite,
                              vk::ImageLayout::eGeneral,
                              vk::AccessFlagBits::eShaderRead);
        mRedistanceBack.PushConstant(commandBuffer, 8, delta);
        mRedistanceBack.Record(commandBuffer);
        Barrier(commandBuffer,
                vk::ImageLayout::eGeneral,
                vk::AccessFlagBits::eShaderWrite,
                vk::ImageLayout::eGeneral,
                vk::AccessFlagBits::eShaderRead);
    });

    mReinitialiseCmd.Record([&](vk::CommandBuffer commandBuffer)
    {
        mLevelSet0.CopyFrom(commandBuffer, *this);

        for (int i = 0; i < reinitializeIterations; i++)
        {
            mRedistanceFront.PushConstant(commandBuffer, 8, delta);
            mRedistanceFront.Record(commandBuffer);
            mLevelSetBack.Barrier(commandBuffer,
                                  vk::ImageLayout::eGeneral,
                                  vk::AccessFlagBits::eShaderWrite,
                                  vk::ImageLayout::eGeneral,
                                  vk::AccessFlagBits::eShaderRead);
            mRedistanceBack.PushConstant(commandBuffer, 8, delta);
            mRedistanceBack.Record(commandBuffer);
            Barrier(commandBuffer,
                    vk::ImageLayout::eGeneral,
                    vk::AccessFlagBits::eShaderWrite,
                    vk::ImageLayout::eGeneral,
                    vk::AccessFlagBits::eShaderRead);
        }
    });
}

void LevelSet::Extrapolate(Renderer::Buffer& solidPhi)
{
    mExtrapolateBound = mExtrapolate.Bind({solidPhi, *this});
    mExtrapolateCmd.Record([&](vk::CommandBuffer commandBuffer)
    {
        // TODO add barrier
        mExtrapolateBound.Record(commandBuffer);
    });
}

void LevelSet::Reinitialise()
{
    mReinitialiseCmd.Submit();
}

void LevelSet::Redistance()
{
    mRedistanceCmd.Submit();
}

void LevelSet::Extrapolate()
{
    mExtrapolateCmd.Submit();
}

}}
