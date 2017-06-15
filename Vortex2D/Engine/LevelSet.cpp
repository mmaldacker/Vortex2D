//
//  LevelSet.cpp
//  Vortex2D
//

#include "LevelSet.h"

namespace Vortex2D { namespace  Fluid {

LevelSet::LevelSet(const Renderer::Device& device, const glm::vec2& size)
    : Renderer::RenderTexture(device, size.x, size.y, vk::Format::eR32Sfloat)
    , mLevelSet0(device, size.x, size.y, vk::Format::eR32Sfloat, false)
    , mLevelSetBack(device, size.x, size.y, vk::Format::eR32Sfloat, false)
{
    // Redistance compute shader
    auto redistanceShader = device.GetShaderModule("../Vortex2D/Redistance.comp.spv");

    static auto redistanceLayout = Renderer::DescriptorSetLayoutBuilder()
            .Binding(0, vk::DescriptorType::eStorageImage, vk::ShaderStageFlagBits::eCompute, 1)
            .Binding(1, vk::DescriptorType::eStorageImage, vk::ShaderStageFlagBits::eCompute, 1)
            .Binding(2, vk::DescriptorType::eStorageImage, vk::ShaderStageFlagBits::eCompute, 1)
            .Create(device);

    mRedistanceFrontDescriptorSet = Renderer::MakeDescriptorSet(device, redistanceLayout);
    mRedistanceBackDescriptorSet = Renderer::MakeDescriptorSet(device, redistanceLayout);

    Renderer::DescriptorSetUpdater(*mRedistanceFrontDescriptorSet)
            .WriteImages(0, 0, vk::DescriptorType::eStorageImage).Image({}, mLevelSet0, vk::ImageLayout::eGeneral)
            .WriteImages(1, 0, vk::DescriptorType::eStorageImage).Image({}, *this, vk::ImageLayout::eGeneral)
            .WriteImages(2, 0, vk::DescriptorType::eStorageImage).Image({}, mLevelSetBack, vk::ImageLayout::eGeneral)
            .Update(device.Handle());

    Renderer::DescriptorSetUpdater(*mRedistanceBackDescriptorSet)
            .WriteImages(0, 0, vk::DescriptorType::eStorageImage).Image({}, mLevelSet0, vk::ImageLayout::eGeneral)
            .WriteImages(1, 0, vk::DescriptorType::eStorageImage).Image({}, mLevelSetBack, vk::ImageLayout::eGeneral)
            .WriteImages(2, 0, vk::DescriptorType::eStorageImage).Image({}, *this, vk::ImageLayout::eGeneral)
            .Update(device.Handle());

    mRedistanceLayout = Renderer::PipelineLayoutBuilder()
            .DescriptorSetLayout(redistanceLayout)
            .Create(device.Handle());

    mRedistancePipeline = Renderer::MakeComputePipeline(device.Handle(), redistanceShader, *mRedistanceLayout);

    // Extrapolate compute shader
    auto extrapolateShader = device.GetShaderModule("../Vortex2D/Extrapolate.comp.spv");

    static auto extrapolateLayout = Renderer::DescriptorSetLayoutBuilder()
            .Binding(0, vk::DescriptorType::eStorageImage, vk::ShaderStageFlagBits::eCompute, 1)
            .Binding(1, vk::DescriptorType::eStorageBuffer, vk::ShaderStageFlagBits::eCompute, 1)
            .Create(device);

    mExtrapolateDescriptorSet = Renderer::MakeDescriptorSet(device, extrapolateLayout);

    mExtrapolateLayout = Renderer::PipelineLayoutBuilder()
            .DescriptorSetLayout(extrapolateLayout)
            .Create(device.Handle());

    mExtrapolatePipeline = Renderer::MakeComputePipeline(device.Handle(), extrapolateShader, *mExtrapolateLayout);
}

void LevelSet::Redistance(int iterations)
{
    /*
    mLevelSet0 = mIdentity(*this);

    for (int i = 0; i < iterations; i++)
    {
        Swap();
        *this = mRedistance(Back(*this), mLevelSet0);
    }
    */
}

void LevelSet::Extrapolate(Renderer::Buffer& solidPhi)
{
    /*
    Swap();
    *this = mExtrapolate(Back(*this), solidPhi);
    */
}

}}
