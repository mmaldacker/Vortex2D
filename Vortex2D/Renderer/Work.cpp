//
//  Work.cpp
//  Vortex2D
//

#include "Work.h"

#include <Vortex2D/Renderer/DescriptorSet.h>
#include <Vortex2D/Renderer/Pipeline.h>
#include <Vortex2D/SPIRV/Reflection.h>

namespace Vortex2D { namespace Renderer {

glm::ivec2 ComputeSize::GetLocalSize2D()
{
    return {64, 4};
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

glm::ivec2 ComputeSize::GetWorkSize(int size)
{
    glm::vec2 localSize(GetLocalSize1D(), 1);
    return glm::ceil(glm::vec2(size, 1) / localSize);
}

ComputeSize::ComputeSize(const glm::ivec2& size)
    : DomainSize(size)
    , WorkSize(GetWorkSize(size))
    , LocalSize(GetLocalSize2D())
{
}

ComputeSize::ComputeSize(int size)
    : DomainSize({size, 1})
    , WorkSize(GetWorkSize(size))
    , LocalSize({GetLocalSize1D(), 1})
{

}

ComputeSize ComputeSize::Default2D()
{
    return ComputeSize({1, 1});
}

ComputeSize ComputeSize::Default1D()
{
    return ComputeSize(1);
}

ComputeSize MakeStencilComputeSize(const glm::ivec2& size, int radius)
{
    ComputeSize computeSize(ComputeSize::Default2D());

    auto localSize = ComputeSize::GetLocalSize2D();
    computeSize.DomainSize = size;
    computeSize.LocalSize = localSize;
    computeSize.WorkSize = glm::ceil(glm::vec2(size) / glm::vec2(localSize - glm::ivec2(2 * radius)));

    return computeSize;
}

ComputeSize MakeCheckerboardComputeSize(const glm::ivec2& size)
{
    auto localSize = ComputeSize::GetLocalSize2D() * glm::ivec2(1, 2);

    ComputeSize computeSize(ComputeSize::Default2D());
    computeSize.DomainSize = size;
    computeSize.LocalSize = localSize;
    computeSize.WorkSize = glm::ceil(glm::vec2(size) / (glm::vec2(localSize) * glm::vec2(2.0f, 1.0f)));

    return computeSize;
}

DispatchParams::DispatchParams(int count)
    : workSize(static_cast<uint32_t>(std::ceil(static_cast<float>(count) / Renderer::ComputeSize::GetLocalSize1D())), 1, 1)
    , count(count)
{
}

Work::Work(const Device& device,
           const ComputeSize& computeSize,
           const SpirvBinary& spirv)
    : mComputeSize(computeSize)
    , mDevice(device)
{
    vk::ShaderModule shaderModule = device.GetShaderModule(spirv);
    SPIRV::Reflection reflection(spirv);
    if (reflection.GetShaderStage() != vk::ShaderStageFlagBits::eCompute) throw std::runtime_error("only compute supported");

    mPipelineLayout = {{reflection}};
    auto layout = device.GetLayoutManager().GetPipelineLayout(mPipelineLayout);

    assert(mComputeSize.LocalSize.x > 0 && mComputeSize.LocalSize.y > 0);
    mPipeline = MakeComputePipeline(device.Handle(), shaderModule, layout, mComputeSize.LocalSize.x, mComputeSize.LocalSize.y);
}

Work::Bound Work::Bind(ComputeSize computeSize, const std::vector<Renderer::BindingInput>& inputs)
{
    if (inputs.size() != mPipelineLayout.layouts.front().bindings.size())
    {
        throw std::runtime_error("Unmatched inputs and bindings");
    }

    auto descriptorSet = mDevice.GetLayoutManager().MakeDescriptorSet(mPipelineLayout);
    Renderer::Bind(mDevice, *descriptorSet.descriptorSet, mPipelineLayout, inputs);

    return Bound(computeSize, mPipelineLayout.layouts.front().pushConstantSize, descriptorSet.pipelineLayout, *mPipeline, std::move(descriptorSet.descriptorSet));
}

Work::Bound Work::Bind(const std::vector<BindingInput>& inputs)
{
    return Bind(mComputeSize, inputs);
}

Work::Bound::Bound()
    : mComputeSize(ComputeSize::Default2D())
    , mLayout(nullptr)
    , mPipeline(nullptr)
{

}

Work::Bound::Bound(const ComputeSize& computeSize,
                   uint32_t pushConstantSize,
                   vk::PipelineLayout layout,
                   vk::Pipeline pipeline,
                   vk::UniqueDescriptorSet descriptor)
    : mComputeSize(computeSize)
    , mPushConstantSize(pushConstantSize)
    , mLayout(layout)
    , mPipeline(pipeline)
    , mDescriptor(std::move(descriptor))
{

}

void Work::Bound::Record(vk::CommandBuffer commandBuffer)
{
    PushConstant(commandBuffer, 0, mComputeSize.DomainSize.x);
    PushConstant(commandBuffer, 4, mComputeSize.DomainSize.y);
    Dispatch(commandBuffer);
}

void Work::Bound::RecordIndirect(vk::CommandBuffer commandBuffer, GenericBuffer& dispatchParams)
{
    PushConstant(commandBuffer, 0, mComputeSize.DomainSize.x);
    PushConstant(commandBuffer, 4, mComputeSize.DomainSize.y);
    commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eCompute, mLayout, 0, {*mDescriptor}, {});
    commandBuffer.bindPipeline(vk::PipelineBindPoint::eCompute, mPipeline);

    commandBuffer.dispatchIndirect(dispatchParams.Handle(), 0);
}

void Work::Bound::Dispatch(vk::CommandBuffer commandBuffer)
{
  commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eCompute, mLayout, 0, {*mDescriptor}, {});
  commandBuffer.bindPipeline(vk::PipelineBindPoint::eCompute, mPipeline);

  commandBuffer.dispatch(mComputeSize.WorkSize.x, mComputeSize.WorkSize.y, 1);
}

}}
