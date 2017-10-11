//
//  DescriptorSet.h
//  Vortex2D
//

#ifndef DescriptorSet_h
#define DescriptorSet_h

#include <Vortex2D/Renderer/Common.h>
#include <Vortex2D/Renderer/Device.h>
#include <Vortex2D/Renderer/Buffer.h>
#include <Vortex2D/Renderer/Texture.h>

#include <Vortex2D/Utils/variant.hpp>
#include <map>

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

using DescriptorTypeBindings = std::map<unsigned, vk::DescriptorType>;

struct DescriptorImage
{
    DescriptorImage(vk::Sampler sampler, Renderer::Texture& texture);
    DescriptorImage(Renderer::Texture& texture);

    vk::Sampler Sampler;
    Renderer::Texture* Texture;
};

struct BindingInput
{
    static constexpr unsigned DefaultBind = -1;

    BindingInput(Renderer::Buffer& buffer, unsigned bind = DefaultBind);
    BindingInput(Renderer::Texture& texture,  unsigned bind = DefaultBind);
    BindingInput(vk::Sampler sampler, Renderer::Texture& texture,  unsigned bind = DefaultBind);

    unsigned Bind;
    mpark::variant<Renderer::Buffer*, DescriptorImage> Input;
};

vk::UniqueDescriptorSet MakeDescriptorSet(const Device& device, vk::DescriptorSetLayout layout);

class DescriptorSetUpdater
{
public:
    DescriptorSetUpdater(vk::DescriptorSet dstSet, int maxBuffers = 10, int maxImages = 10);

    DescriptorSetUpdater& Bind(DescriptorTypeBindings bindings, const std::vector<BindingInput>& bindingInputs);

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
