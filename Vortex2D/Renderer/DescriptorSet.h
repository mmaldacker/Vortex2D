//
//  DescriptorSet.h
//  Vortex2D
//

#ifndef DescriptorSet_h
#define DescriptorSet_h

#include <Vortex2D/Renderer/Common.h>
#include <Vortex2D/Renderer/Device.h>
#include <Vortex2D/Renderer/Buffer.h>

namespace Vortex2D { namespace Renderer {

class DescriptorSetLayoutBuilder
{
public:
    DescriptorSetLayoutBuilder& Binding(uint32_t binding,
                                        vk::DescriptorType descriptorType,
                                        vk::ShaderStageFlags stageFlags,
                                        uint32_t descriptorCount);

    vk::DescriptorSetLayout Create(const Device& device);

private:
    std::vector<vk::DescriptorSetLayoutBinding> mDescriptorSetLayoutBindings;
};

class DescriptorSet
{
public:
    DescriptorSet() = default;
    DescriptorSet(vk::Device device, vk::DescriptorSetLayout layout, vk::DescriptorPool pool);

    operator vk::DescriptorSet() const;

private:
    vk::UniqueDescriptorSet mDescriptorSet;
};

// TODO this can be merged with the DescriptorSet class
class DescriptorSetUpdater
{
public:
    DescriptorSetUpdater(int maxBuffers = 10, int maxImages = 10);

    DescriptorSetUpdater& WriteDescriptorSet(vk::DescriptorSet dstSet);
    DescriptorSetUpdater& WriteImages(uint32_t dstBinding, uint32_t dstArrayElement, vk::DescriptorType descriptorType);
    DescriptorSetUpdater& Image(vk::Sampler sampler, vk::ImageView imageView, vk::ImageLayout imageLayout);
    DescriptorSetUpdater& WriteBuffers(uint32_t dstBinding, uint32_t dstArrayElement, vk::DescriptorType descriptorType);
    DescriptorSetUpdater& Buffer(const Buffer& buffer);

    void Update(vk::Device device) const;

private:
    std::vector<vk::DescriptorBufferInfo> mBufferInfo;
    std::vector<vk::DescriptorImageInfo> mImageInfo;
    std::vector<vk::WriteDescriptorSet> mDescriptorWrites;
    vk::DescriptorSet mDstSet;
    int mNumBuffers;
    int mNumImages;
};

}}

#endif
