//
//  LevelSet.cpp
//  Vortex2D
//

#include "LevelSet.h"

#include <Vortex2D/Renderer/CommandBuffer.h>

namespace Vortex2D { namespace  Fluid {

LevelSet::LevelSet(const Renderer::Device& device, const glm::vec2& size)
    : Renderer::RenderTexture(device, size.x, size.y, vk::Format::eR32Sfloat)
    , mLevelSet0(device, size.x, size.y, vk::Format::eR32Sfloat, false)
    , mLevelSetBack(device, size.x, size.y, vk::Format::eR32Sfloat, false)
    , mExtrapolate(device, size, "../Vortex2D/Extrapolate.comp.spv",
                   {vk::DescriptorType::eStorageImage,
                    vk::DescriptorType::eStorageImage})
    , mRedistance(device, size, "../Vortex2D/Redistance.comp.spv",
                  {vk::DescriptorType::eStorageImage,
                   vk::DescriptorType::eStorageImage,
                   vk::DescriptorType::eStorageImage})
    , mRedistanceFront(mRedistance.Bind({mLevelSet0, *this, mLevelSetBack}))
    , mRedistanceBack(mRedistance.Bind({mLevelSet0, mLevelSetBack, *this}))
    , mExtrapolateCmd(device)
    , mReinitialiseCmd(device)
    , mRedistanceCmd(device)
{
    const float delta = 0.1f;

    mRedistanceCmd.Wait();
    mRedistanceCmd.Record([&](vk::CommandBuffer commandBuffer)
    {
        // TODO add barriers
        mLevelSet0.CopyFrom(commandBuffer, *this);
        mRedistanceFront.PushConstant(commandBuffer, 8, delta);
        mRedistanceFront.Record(commandBuffer);
        mRedistanceBack.PushConstant(commandBuffer, 8, delta);
        mRedistanceBack.Record(commandBuffer);
    });

    mReinitialiseCmd.Wait();
    mReinitialiseCmd.Record([&](vk::CommandBuffer commandBuffer)
    {
        // TODO add barriers
        mLevelSet0.CopyFrom(commandBuffer, *this);

        mRedistanceFront.PushConstant(commandBuffer, 8, delta);
        mRedistanceBack.PushConstant(commandBuffer, 8, delta);

        for (int i = 0; i < 50; i++)
        {
            mRedistanceFront.Record(commandBuffer);
            mRedistanceBack.Record(commandBuffer);
        }
    });
}

void LevelSet::Extrapolate(Renderer::Buffer& solidPhi)
{
    mExtrapolateBound = mExtrapolate.Bind({solidPhi, *this});
    mExtrapolateCmd.Wait();
    mExtrapolateCmd.Record([&](vk::CommandBuffer commandBuffer)
    {
        // TODO add barrier
        mExtrapolateBound.Record(commandBuffer);
    });
}

}}
