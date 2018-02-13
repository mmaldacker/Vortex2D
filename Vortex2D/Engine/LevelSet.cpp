//
//  LevelSet.cpp
//  Vortex2D
//

#include "LevelSet.h"

#include <Vortex2D/Engine/Boundaries.h>
#include <Vortex2D/Renderer/CommandBuffer.h>

#include "vortex2d_generated_spirv.h"

namespace Vortex2D { namespace  Fluid {

LevelSet::LevelSet(const Renderer::Device& device, const glm::ivec2& size, int reinitializeIterations)
    : Renderer::RenderTexture(device, size.x, size.y, vk::Format::eR32Sfloat)
    , mLevelSet0(device, size.x, size.y, vk::Format::eR32Sfloat, false)
    , mLevelSetBack(device, size.x, size.y, vk::Format::eR32Sfloat, false)
    , mSampler(Renderer::SamplerBuilder()
               .AddressMode(vk::SamplerAddressMode::eClampToEdge)
               .Create(device.Handle()))
    , mExtrapolate(device, size, Extrapolate_comp)
    , mRedistance(device, size, Redistance_comp)
    , mRedistanceFront(mRedistance.Bind({{*mSampler, mLevelSet0}, {*mSampler, *this}, mLevelSetBack}))
    , mRedistanceBack(mRedistance.Bind({{*mSampler, mLevelSet0}, {*mSampler, mLevelSetBack}, *this}))
    , mExtrapolateCmd(device, false)
    , mReinitialiseCmd(device, false)
    , mSignedObjectCmd(device, false)
{
    mReinitialiseCmd.Record([&, reinitializeIterations](vk::CommandBuffer commandBuffer)
    {
        commandBuffer.debugMarkerBeginEXT({"Reinitialise", {{ 0.98f, 0.49f, 0.26f, 1.0f}}});

        mLevelSet0.CopyFrom(commandBuffer, *this);

        for (int i = 0; i < reinitializeIterations / 2; i++)
        {
            mRedistanceFront.PushConstant(commandBuffer, 8, 0.1f);
            mRedistanceFront.Record(commandBuffer);
            mLevelSetBack.Barrier(commandBuffer,
                                  vk::ImageLayout::eGeneral,
                                  vk::AccessFlagBits::eShaderWrite,
                                  vk::ImageLayout::eGeneral,
                                  vk::AccessFlagBits::eShaderRead);
            mRedistanceBack.PushConstant(commandBuffer, 8, 0.1f);
            mRedistanceBack.Record(commandBuffer);
            Barrier(commandBuffer,
                    vk::ImageLayout::eGeneral,
                    vk::AccessFlagBits::eShaderWrite,
                    vk::ImageLayout::eGeneral,
                    vk::AccessFlagBits::eShaderRead);
        }

        commandBuffer.debugMarkerEndEXT();
    });
}

void LevelSet::ExtrapolateInit(Renderer::Texture& solidPhi)
{
    mExtrapolateBound = mExtrapolate.Bind({solidPhi, *this});
    mExtrapolateCmd.Record([&](vk::CommandBuffer commandBuffer)
    {
        commandBuffer.debugMarkerBeginEXT({"Extrapolate phi", {{ 0.53f, 0.09f, 0.16f, 1.0f}}});
        ExtrapolateRecord(commandBuffer);
        commandBuffer.debugMarkerEndEXT();
    });
}

void LevelSet::ExtrapolateRecord(vk::CommandBuffer commandBuffer)
{
    mExtrapolateBound.Record(commandBuffer);
    Barrier(commandBuffer,
            vk::ImageLayout::eGeneral,
            vk::AccessFlagBits::eShaderWrite,
            vk::ImageLayout::eGeneral,
            vk::AccessFlagBits::eShaderRead);
}

void LevelSet::Reinitialise()
{
    mReinitialiseCmd.Submit();
}

void LevelSet::Extrapolate()
{
    mExtrapolateCmd.Submit();
}

}}
