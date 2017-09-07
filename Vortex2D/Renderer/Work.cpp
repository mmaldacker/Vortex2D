//
//  Work.cpp
//  Vortex2D
//

#include "Work.h"

#include <Vortex2D/Renderer/DescriptorSet.h>
#include <Vortex2D/Renderer/Pipeline.h>

namespace Vortex2D { namespace Renderer {

template <typename... Lambdas>
struct lambda_visitor : Lambdas...
{
    lambda_visitor(Lambdas... lambdas) : Lambdas(lambdas)... {}
};

template <typename... Lambdas>
lambda_visitor<Lambdas...> make_lambda_visitor(Lambdas... lambdas)
{
    return { lambdas... };
}

glm::ivec2 ComputeSize::GetLocalSize2D()
{
    return {16, 16};
}

int ComputeSize::GetLocalSize1D()
{
    return 256;
}

glm::ivec2 ComputeSize::GetWorkSize(const glm::ivec2& size)
{
    glm::vec2 localSize = GetLocalSize2D();
    return glm::ceil(glm::vec2(size) / localSize);
}

ComputeSize::ComputeSize()
    : DomainSize(0)
    , WorkSize(0)
    , LocalSize(0)
{
}

ComputeSize::ComputeSize(const glm::ivec2& size)
    : DomainSize(size)
    , WorkSize(GetWorkSize(size))
    , LocalSize(GetLocalSize2D())
{
}

ComputeSize MakeStencilComputeSize(const glm::ivec2& size, int radius)
{
    ComputeSize computeSize;

    auto localSize = ComputeSize::GetLocalSize2D();
    computeSize.DomainSize = size;
    computeSize.LocalSize = localSize;
    computeSize.WorkSize = glm::ceil(glm::vec2(size) / glm::vec2(localSize - glm::ivec2(2 * radius)));

    return computeSize;
}

ComputeSize MakeCheckerboardComputeSize(const glm::ivec2& size)
{
    auto localSize = ComputeSize::GetLocalSize2D() * glm::ivec2(1, 2);

    ComputeSize computeSize;
    computeSize.DomainSize = size;
    computeSize.LocalSize = localSize;
    computeSize.WorkSize = glm::ceil(glm::vec2(size) / (glm::vec2(localSize) * glm::vec2(2.0f, 1.0f)));

    return computeSize;
}

Work::Input::Input(Renderer::Buffer& buffer)
    : Bind(&buffer)
{
}

Work::Input::Input(Renderer::Texture& texture)
    : Bind(DescriptorImage(texture))
{
}

Work::Input::Input(vk::Sampler sampler, Renderer::Texture& texture)
    : Bind(DescriptorImage(sampler, texture))
{
}

Work::Input::DescriptorImage::DescriptorImage(vk::Sampler sampler, Renderer::Texture& texture)
    : Sampler(sampler)
    , Texture(&texture)
{
}

Work::Input::DescriptorImage::DescriptorImage(Renderer::Texture& texture)
    : Sampler()
    , Texture(&texture)
{
}

Work::Work(const Device& device,
           const ComputeSize& computeSize,
           const std::string& shader,
           const std::vector<vk::DescriptorType>& binding,
           const uint32_t pushConstantExtraSize)
    : mComputeSize(computeSize)
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

    mPipeline = MakeComputePipeline(device.Handle(), shaderModule, *mLayout, mComputeSize.LocalSize.x, mComputeSize.LocalSize.y);
}

Work::Bound Work::Bind(const std::vector<Input>& inputs)
{
    if (inputs.size() != mBindings.size())
    {
        throw std::runtime_error("Unmatched inputs and bindings");
    }

    vk::UniqueDescriptorSet descriptor = MakeDescriptorSet(mDevice, mDescriptorLayout);

    DescriptorSetUpdater updater(*descriptor);
    for (int i = 0; i < inputs.size(); i++)
    {
        auto visitor = make_lambda_visitor(
        [&](Renderer::Buffer* buffer)
        {
            if (mBindings[i] != vk::DescriptorType::eStorageBuffer) throw std::runtime_error("Binding not a storage buffer");

            updater.WriteBuffers(i, 0, mBindings[i]).Buffer(*buffer);
        },
        [&](Input::DescriptorImage image)
        {
            if (mBindings[i] != vk::DescriptorType::eStorageImage &&
                mBindings[i] != vk::DescriptorType::eCombinedImageSampler) throw std::runtime_error("Binding not an image");

            updater.WriteImages(i, 0, mBindings[i]).Image(image.Sampler, *image.Texture, vk::ImageLayout::eGeneral);
        },
        [&](auto& other)
        {
            throw std::runtime_error("Binding not supported");
        });

        std::visit(visitor, inputs[i].Bind);
    }
    updater.Update(mDevice.Handle());

    return Bound(mComputeSize, *mLayout, *mPipeline, std::move(descriptor));
}

Work::Bound::Bound(const ComputeSize& computeSize,
                   vk::PipelineLayout layout,
                   vk::Pipeline pipeline,
                   vk::UniqueDescriptorSet descriptor)
    : mComputeSize(computeSize)
    , mLayout(layout)
    , mPipeline(pipeline)
    , mDescriptor(std::move(descriptor))
{

}

void Work::Bound::Record(vk::CommandBuffer commandBuffer)
{
    commandBuffer.pushConstants(mLayout, vk::ShaderStageFlagBits::eCompute, 0, 4, &mComputeSize.DomainSize.x);
    commandBuffer.pushConstants(mLayout, vk::ShaderStageFlagBits::eCompute, 4, 4, &mComputeSize.DomainSize.y);
    commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eCompute, mLayout, 0, {*mDescriptor}, {});
    commandBuffer.bindPipeline(vk::PipelineBindPoint::eCompute, mPipeline);

    commandBuffer.dispatch(mComputeSize.WorkSize.x, mComputeSize.WorkSize.y, 1);
}

}}
