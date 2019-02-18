//
//  DescriptorSet.cpp
//  Vortex2D
//

#include "DescriptorSet.h"

#include <Vortex2D/Renderer/Device.h>
#include <Vortex2D/Renderer/Buffer.h>
#include <Vortex2D/Renderer/Texture.h>
#include <Vortex2D/SPIRV/Reflection.h>

namespace Vortex2D { namespace Renderer {

bool operator==(const ShaderLayout& left, const ShaderLayout& right)
{
    return left.bindings == right.bindings &&
           left.pushConstantSize == right.pushConstantSize &&
           left.shaderStage == right.shaderStage;
}

bool operator==(const PipelineLayout& left, const PipelineLayout& right)
{
    return left.layouts == right.layouts;
}

ShaderLayout::ShaderLayout(const SPIRV::Reflection& reflection)
    : shaderStage(reflection.GetShaderStage())
    , bindings(reflection.GetDescriptorTypesMap())
    , pushConstantSize(reflection.GetPushConstantsSize())
{
}

LayoutManager::LayoutManager(const Device& device)
    : mDevice(device)
{
}

void LayoutManager::CreateDescriptorPool()
{
    // create descriptor pool
    // TODO size should be configurable
    // TODO check when we allocate more than what is allowed (might get it for free already)
    std::vector<vk::DescriptorPoolSize> poolSizes;
    poolSizes.emplace_back(vk::DescriptorType::eUniformBuffer, 512);
    poolSizes.emplace_back(vk::DescriptorType::eCombinedImageSampler, 512);
    poolSizes.emplace_back(vk::DescriptorType::eStorageImage, 512);
    poolSizes.emplace_back(vk::DescriptorType::eStorageBuffer, 512);

    vk::DescriptorPoolCreateInfo descriptorPoolInfo{};
    descriptorPoolInfo.flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet;
    descriptorPoolInfo.maxSets = 512;
    descriptorPoolInfo.poolSizeCount = (uint32_t)poolSizes.size();
    descriptorPoolInfo.pPoolSizes = poolSizes.data();
    mDescriptorPool = mDevice.Handle().createDescriptorPoolUnique(descriptorPoolInfo);
}

vk::DescriptorSetLayout LayoutManager::GetDescriptorSetLayout(const PipelineLayout& layout)
{
    auto it = std::find_if(mDescriptorSetLayouts.begin(), mDescriptorSetLayouts.end(),
                           [&](const auto& descriptorSetLayout)
    {
        return std::get<0>(descriptorSetLayout) == layout;
    });

    if (it == mDescriptorSetLayouts.end())
    {
        std::vector<vk::DescriptorSetLayoutBinding> descriptorSetLayoutBindings;
        for (auto& shaderLayout: layout.layouts)
        {
            for (auto& desciptorType: shaderLayout.bindings)
            {
                descriptorSetLayoutBindings.push_back({desciptorType.first,
                                                       desciptorType.second,
                                                       1,
                                                       shaderLayout.shaderStage,
                                                       nullptr});
            }
        }

        auto descriptorSetLayoutInfo = vk::DescriptorSetLayoutCreateInfo()
                .setBindingCount((uint32_t)descriptorSetLayoutBindings.size())
                .setPBindings(descriptorSetLayoutBindings.data());

        auto descriptorSetLayout = mDevice.Handle().createDescriptorSetLayoutUnique(descriptorSetLayoutInfo);
        mDescriptorSetLayouts.emplace_back(layout, std::move(descriptorSetLayout));
        return *std::get<1>(mDescriptorSetLayouts.back());
    }

    return *std::get<1>(*it);
}

vk::PipelineLayout LayoutManager::GetPipelineLayout(const PipelineLayout& layout)
{
    auto it = std::find_if(mPipelineLayouts.begin(), mPipelineLayouts.end(),
                           [&](const auto& pipelineLayout)
    {
        return std::get<0>(pipelineLayout) == layout;
    });

    if (it == mPipelineLayouts.end())
    {
        vk::DescriptorSetLayout descriptorSetlayouts[] = {GetDescriptorSetLayout(layout)};
        std::vector<vk::PushConstantRange> pushConstantRanges;
        uint32_t totalPushConstantSize = 0;
        for (auto& shaderLayout: layout.layouts)
        {
            if (shaderLayout.pushConstantSize > 0)
            {
                pushConstantRanges.push_back({shaderLayout.shaderStage, totalPushConstantSize, shaderLayout.pushConstantSize});
                totalPushConstantSize += shaderLayout.pushConstantSize;
            }
        }

        auto pipelineLayoutInfo = vk::PipelineLayoutCreateInfo()
                .setSetLayoutCount(1)
                .setPSetLayouts(descriptorSetlayouts);

        if (totalPushConstantSize > 0)
        {
            pipelineLayoutInfo
                    .setPPushConstantRanges(pushConstantRanges.data())
                    .setPushConstantRangeCount((uint32_t)pushConstantRanges.size());
        }

        mPipelineLayouts.emplace_back(layout, mDevice.Handle().createPipelineLayoutUnique(pipelineLayoutInfo));
        return *std::get<1>(mPipelineLayouts.back());
    }

    return *std::get<1>(*it);
}

DescriptorSet LayoutManager::MakeDescriptorSet(const PipelineLayout& layout)
{
    vk::DescriptorSetLayout descriptorSetlayouts[] = {GetDescriptorSetLayout(layout)};

    auto descriptorSetInfo = vk::DescriptorSetAllocateInfo()
            .setDescriptorPool(*mDescriptorPool)
            .setDescriptorSetCount(1)
            .setPSetLayouts(descriptorSetlayouts);

    DescriptorSet descriptorSet;
    descriptorSet.descriptorSet = std::move(mDevice.Handle().allocateDescriptorSetsUnique(descriptorSetInfo).at(0));
    descriptorSet.descriptorSetLayout = GetDescriptorSetLayout(layout);
    descriptorSet.pipelineLayout = GetPipelineLayout(layout);

    return descriptorSet;
}

BindingInput::BindingInput(Renderer::GenericBuffer& buffer, uint32_t bind)
    : Bind(bind)
    , Input(&buffer)

{
}

BindingInput::BindingInput(Renderer::Texture& texture, uint32_t bind)
    : Bind(bind)
    , Input(DescriptorImage(texture))
{
}

BindingInput::BindingInput(vk::Sampler sampler, Renderer::Texture& texture, uint32_t bind)
    : Bind(bind)
    , Input(DescriptorImage(sampler, texture))
{
}

DescriptorImage::DescriptorImage(vk::Sampler sampler, Renderer::Texture& texture)
    : Sampler(sampler)
    , Texture(&texture)
{
}

DescriptorImage::DescriptorImage(Renderer::Texture& texture)
    : Sampler()
    , Texture(&texture)
{
}

vk::DescriptorType GetDescriptorType(uint32_t bind, const PipelineLayout& layout)
{
    for (auto& shaderLayout: layout.layouts)
    {
        auto it = shaderLayout.bindings.find(bind);
        if (it != shaderLayout.bindings.end())
        {
            return it->second;
        }
    }

    throw std::runtime_error("no bindings defined");
}

void Bind(const Device& device, DescriptorSet& dstSet, const PipelineLayout& layout, const std::vector<BindingInput>& bindingInputs)
{
    std::vector<vk::DescriptorBufferInfo> bufferInfo(20);
    std::vector<vk::DescriptorImageInfo> imageInfo(20);
    std::vector<vk::WriteDescriptorSet> descriptorWrites;
    std::size_t numBuffers = 0;
    std::size_t numImages = 0;

    for (std::size_t i = 0; i < bindingInputs.size(); i++)
    {
        bindingInputs[i].Input.match(
        [&](Renderer::GenericBuffer* buffer)
        {
            uint32_t bind = bindingInputs[i].Bind == BindingInput::DefaultBind ? static_cast<uint32_t>(i) : bindingInputs[i].Bind;

            auto descriptorType = GetDescriptorType(bind, layout);
            if (descriptorType != vk::DescriptorType::eStorageBuffer &&
            descriptorType != vk::DescriptorType::eUniformBuffer) throw std::runtime_error("Binding not a storage buffer");

            auto writeDescription = vk::WriteDescriptorSet()
                .setDstSet(*dstSet.descriptorSet)
                .setDstBinding(bind)
                .setDstArrayElement(0)
                .setDescriptorType(descriptorType)
                .setPBufferInfo(bufferInfo.data() + numBuffers);
            descriptorWrites.push_back(writeDescription);

            if (!descriptorWrites.empty() && numBuffers != bufferInfo.size() && descriptorWrites.back().pBufferInfo)
            {
                descriptorWrites.back().descriptorCount++;
                bufferInfo[numBuffers++] = vk::DescriptorBufferInfo(buffer->Handle(), 0, buffer->Size());
            }
            else
            {
                assert(false);
            }
        },
        [&](DescriptorImage image)
        {
            uint32_t bind = bindingInputs[i].Bind == BindingInput::DefaultBind ? static_cast<uint32_t>(i) : bindingInputs[i].Bind;

            auto descriptorType = GetDescriptorType(bind, layout);
            if (descriptorType != vk::DescriptorType::eStorageImage &&
            descriptorType != vk::DescriptorType::eCombinedImageSampler) throw std::runtime_error("Binding not an image");

            auto writeDescription = vk::WriteDescriptorSet()
                .setDstSet(*dstSet.descriptorSet)
                .setDstBinding(bind)
                .setDstArrayElement(0)
                .setDescriptorType(descriptorType)
                .setPImageInfo(imageInfo.data() + numImages);
            descriptorWrites.push_back(writeDescription);

            if (!descriptorWrites.empty() && numImages != imageInfo.size() && descriptorWrites.back().pImageInfo)
            {
                descriptorWrites.back().descriptorCount++;
                imageInfo[numImages++] = vk::DescriptorImageInfo(image.Sampler, image.Texture->GetView(), vk::ImageLayout::eGeneral);
            }
            else
            {
                assert(false);
            }
        });
    }

    device.Handle().updateDescriptorSets(descriptorWrites, {});
}

}}
