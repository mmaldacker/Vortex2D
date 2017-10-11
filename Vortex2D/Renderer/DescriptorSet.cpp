//
//  DescriptorSet.cpp
//  Vortex2D
//

#include "DescriptorSet.h"

namespace Vortex2D { namespace Renderer {

namespace
{

template <class... Fs>
struct overload;

template <class F0, class... Frest>
struct overload<F0, Frest...> : F0, overload<Frest...>
{
    overload(F0 f0, Frest... rest) : F0(f0), overload<Frest...>(rest...) {}

    using F0::operator();
    using overload<Frest...>::operator();
};

template <class F0>
struct overload<F0> : F0
{
    overload(F0 f0) : F0(f0) {}

    using F0::operator();
};

template <class... Fs>
auto make_visitor(Fs... fs)
{
    return overload<Fs...>(fs...);
}

}

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
            .setBindingCount((uint32_t)mDescriptorSetLayoutBindings.size())
            .setPBindings(mDescriptorSetLayoutBindings.data());

    return device.CreateDescriptorSetLayout(descriptorSetLayoutInfo);
}

vk::UniqueDescriptorSet MakeDescriptorSet(const Device& device, vk::DescriptorSetLayout layout)
{
    vk::DescriptorSetLayout layouts[] = {layout};
    auto descriptorSetInfo = vk::DescriptorSetAllocateInfo()
            .setDescriptorPool(device.DescriptorPool())
            .setDescriptorSetCount(1)
            .setPSetLayouts(layouts);

    return std::move(device.Handle().allocateDescriptorSetsUnique(descriptorSetInfo).at(0));
}

BindingInput::BindingInput(Renderer::Buffer& buffer, unsigned bind)
    : Bind(bind)
    , Input(&buffer)

{
}

BindingInput::BindingInput(Renderer::Texture& texture, unsigned bind)
    : Bind(bind)
    , Input(DescriptorImage(texture))
{
}

BindingInput::BindingInput(vk::Sampler sampler, Renderer::Texture& texture, unsigned bind)
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

DescriptorSetUpdater::DescriptorSetUpdater(vk::DescriptorSet dstSet, int maxBuffers, int maxImages)
    : mDstSet(dstSet)
    , mNumBuffers(0)
    , mNumImages(0)
{
    // we must pre-size these buffers as we take pointers to their members.
    mBufferInfo.resize(maxBuffers);
    mImageInfo.resize(maxImages);
}

DescriptorSetUpdater& DescriptorSetUpdater::Bind(DescriptorTypeBindings bindings, const std::vector<BindingInput>& bindingInputs)
{
    for (int i = 0; i < bindingInputs.size(); i++)
    {
        auto visitor = make_visitor(
        [&](Renderer::Buffer* buffer)
        {
            unsigned bind = bindingInputs[i].Bind == BindingInput::DefaultBind ? i : bindingInputs[i].Bind;

            if (bindings.count(bind) == 0) throw std::runtime_error("no binding defined");
            if (bindings[bind] != vk::DescriptorType::eStorageBuffer &&
                bindings[bind] != vk::DescriptorType::eUniformBuffer) throw std::runtime_error("Binding not a storage buffer");

            WriteBuffers(bind, 0, bindings[bind]).Buffer(*buffer);
        },
        [&](DescriptorImage image)
        {
            unsigned bind = bindingInputs[i].Bind == BindingInput::DefaultBind ? i : bindingInputs[i].Bind;

            if (bindings.count(bind) == 0) throw std::runtime_error("no binding defined");
            if (bindings[bind] != vk::DescriptorType::eStorageImage &&
                bindings[bind] != vk::DescriptorType::eCombinedImageSampler) throw std::runtime_error("Binding not an image");

            WriteImages(bind, 0, bindings[bind]).Image(image.Sampler, image.Texture->View(), vk::ImageLayout::eGeneral);
        });

        mpark::visit(visitor, bindingInputs[i].Input);
    }

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
    if (!mDescriptorWrites.empty() && mNumImages != mImageInfo.size() && mDescriptorWrites.back().pImageInfo)
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
