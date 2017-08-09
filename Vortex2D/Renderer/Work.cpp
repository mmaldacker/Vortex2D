//
//  Work.cpp
//  Vortex2D
//

#include "Work.h"

#include <Vortex2D/Renderer/DescriptorSet.h>
#include <Vortex2D/Renderer/Pipeline.h>

namespace Vortex2D { namespace Renderer {

Work::Input::Input(Renderer::Buffer& buffer)
    : Buffer(&buffer)
{
}

Work::Input::Input(Renderer::Texture& texture)
    : Image(texture)
{
}

Work::Input::Input(vk::Sampler sampler, Renderer::Texture& texture)
    : Image(sampler, texture)
{
}


Work::Work(const Device& device,
           const glm::ivec2& size,
           const std::string& shader,
           const std::vector<vk::DescriptorType>& binding,
           const uint32_t pushConstantExtraSize)
    : mWidth(size.x)
    , mHeight(size.y)
    , mDevice(device)
    , mBindings(binding)
{
    vk::ShaderModule shaderModule = device.GetShaderModule(shader);

    DescriptorSetLayoutBuilder layoutBuilder;
    for (int i = 0; i < binding.size(); i++)
    {
        layoutBuilder.Binding(i, binding[i], vk::ShaderStageFlagBits::eCompute, 1);
    }
    mDescriptorLayout = layoutBuilder.Create(device);

    mLayout = PipelineLayoutBuilder()
            .DescriptorSetLayout(mDescriptorLayout)
            .PushConstantRange({vk::ShaderStageFlagBits::eCompute, 0, 8 + pushConstantExtraSize})
            .Create(device.Handle());

    auto localSize = GetLocalSize(mWidth, mHeight);
    mPipeline = MakeComputePipeline(device.Handle(), shaderModule, *mLayout, localSize.x, localSize.y);
}

Work::Bound Work::Bind(const std::vector<Input>& inputs)
{
    return Bind({mWidth, mHeight}, inputs);
}

Work::Bound Work::Bind(const glm::ivec2& size, const std::vector<Input>& inputs)
{
    assert(inputs.size() == mBindings.size());

    vk::UniqueDescriptorSet descriptor = MakeDescriptorSet(mDevice, mDescriptorLayout);

    DescriptorSetUpdater updater(*descriptor);
    for (int i = 0; i < inputs.size(); i++)
    {
        if (mBindings[i] == vk::DescriptorType::eStorageBuffer)
        {
            updater.WriteBuffers(i, 0, mBindings[i]).Buffer(*inputs[i].Buffer);
        }
        else if (mBindings[i] == vk::DescriptorType::eStorageImage ||
                 mBindings[i] == vk::DescriptorType::eCombinedImageSampler)
        {
            updater.WriteImages(i, 0, mBindings[i]).Image(inputs[i].Image.Sampler,
                                                          *inputs[i].Image.Texture,
                                                          vk::ImageLayout::eGeneral);
        }
        else
        {
            assert(false);
        }
    }
    updater.Update(mDevice.Handle());

    return Bound(size.x, size.y, *mLayout, *mPipeline, std::move(descriptor));
}

Work::Bound::Bound(uint32_t width,
                   uint32_t height,
                   vk::PipelineLayout layout,
                   vk::Pipeline pipeline,
                   vk::UniqueDescriptorSet descriptor)
    : mWidth(width)
    , mHeight(height)
    , mLayout(layout)
    , mPipeline(pipeline)
    , mDescriptor(std::move(descriptor))
{

}

void Work::Bound::Record(vk::CommandBuffer commandBuffer)
{
    commandBuffer.pushConstants(mLayout, vk::ShaderStageFlagBits::eCompute, 0, 4, &mWidth);
    commandBuffer.pushConstants(mLayout, vk::ShaderStageFlagBits::eCompute, 4, 4, &mHeight);
    commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eCompute, mLayout, 0, {*mDescriptor}, {});
    commandBuffer.bindPipeline(vk::PipelineBindPoint::eCompute, mPipeline);

    auto workSize = GetWorkSize(mWidth, mHeight);
    commandBuffer.dispatch(workSize.x, workSize.y, 1);
}

}}
