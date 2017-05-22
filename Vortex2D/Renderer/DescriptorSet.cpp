//
//  DescriptorSet.cpp
//  Vortex2D
//

#include "DescriptorSet.h"

namespace Vortex2D { namespace Renderer {

DescriptorSetLayoutBuilder& DescriptorSetLayoutBuilder::Binding(uint32_t binding,
                                                                vk::DescriptorType descriptorType,
                                                                vk::ShaderStageFlags stageFlags,
                                                                uint32_t descriptorCount)
{
    mDescriptorSetLayoutBindings.push_back({binding,
                                            descriptorType,
                                            descriptorCount,
                                            stageFlags,
                                            nullptr});
    return *this;
}

vk::DescriptorSetLayout DescriptorSetLayoutBuilder::Create(const Device& device)
{
    auto descriptorSetLayoutInfo = vk::DescriptorSetLayoutCreateInfo()
            .setBindingCount(mDescriptorSetLayoutBindings.size())
            .setPBindings(mDescriptorSetLayoutBindings.data());

    return device.CreateDescriptorSetLayout(descriptorSetLayoutInfo);
}

DescriptorSet::DescriptorSet(vk::Device device, vk::DescriptorSetLayout layout, vk::DescriptorPool pool)
{
    vk::DescriptorSetLayout layouts[] = {layout};
    auto descriptorSetInfo = vk::DescriptorSetAllocateInfo()
            .setDescriptorPool(pool)
            .setDescriptorSetCount(1)
            .setPSetLayouts(layouts);

    mDescriptorSet = std::move(device.allocateDescriptorSetsUnique(descriptorSetInfo).at(0));
}

DescriptorSet::operator vk::DescriptorSet() const
{
    return *mDescriptorSet;
}

DescriptorSetUpdater::DescriptorSetUpdater(int maxBuffers, int maxImages)
    : mNumBuffers(0)
    , mNumImages(0)
{
    // we must pre-size these buffers as we take pointers to their members.
    mBufferInfo.resize(maxBuffers);
    mImageInfo.resize(maxImages);
}

DescriptorSetUpdater& DescriptorSetUpdater::WriteDescriptorSet(vk::DescriptorSet dstSet)
{
    mDstSet = dstSet;

    return *this;
}

DescriptorSetUpdater& DescriptorSetUpdater::WriteImages(uint32_t dstBinding,
                                                        uint32_t dstArrayElement,
                                                        vk::DescriptorType descriptorType)
{
    auto writeDescription = vk::WriteDescriptorSet()
            .setDstSet(mDstSet)
            .setDstBinding(dstBinding)
            .setDstArrayElement(dstArrayElement)
            .setDescriptorType(descriptorType)
            .setPImageInfo(mImageInfo.data() + mNumImages);
    mDescriptorWrites.push_back(writeDescription);

    return *this;
}

DescriptorSetUpdater& DescriptorSetUpdater::Image(vk::Sampler sampler,
                                                  vk::ImageView imageView,
                                                  vk::ImageLayout imageLayout)
{
    if (!mDescriptorWrites.empty() && mNumImages != mImageInfo.size() &&
            mDescriptorWrites.back().pImageInfo)
    {
        mDescriptorWrites.back().descriptorCount++;
        mImageInfo[mNumImages++] = vk::DescriptorImageInfo(sampler, imageView, imageLayout);
    }
    else
    {
        assert(false);
    }

    return *this;
}

DescriptorSetUpdater& DescriptorSetUpdater::WriteBuffers(uint32_t dstBinding,
                                                         uint32_t dstArrayElement,
                                                         vk::DescriptorType descriptorType)
{
    auto writeDescription = vk::WriteDescriptorSet()
            .setDstSet(mDstSet)
            .setDstBinding(dstBinding)
            .setDstArrayElement(dstArrayElement)
            .setDescriptorType(descriptorType)
            .setPBufferInfo(mBufferInfo.data() + mNumBuffers);
    mDescriptorWrites.push_back(writeDescription);

    return *this;
}

DescriptorSetUpdater& DescriptorSetUpdater::Buffer(const ::Vortex2D::Renderer::Buffer& buffer)
{
    if (!mDescriptorWrites.empty() && mNumBuffers != mBufferInfo.size() && mDescriptorWrites.back().pBufferInfo)
    {
        mDescriptorWrites.back().descriptorCount++;
        mBufferInfo[mNumBuffers++] = vk::DescriptorBufferInfo(buffer, 0, buffer.Size());
    }
    else
    {
        assert(false);
    }

    return *this;
}

void DescriptorSetUpdater::Update(vk::Device device) const
{
    device.updateDescriptorSets(mDescriptorWrites, {});
}

}}
