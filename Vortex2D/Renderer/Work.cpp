//
//  Work.cpp
//  Vortex2D
//

#include "Work.h"

#include <Vortex2D/Renderer/DescriptorSet.h>
#include <Vortex2D/Renderer/Pipeline.h>

namespace Vortex2D { namespace Renderer {

Work::Work(const Device& device,
           const glm::vec2& size,
           const std::string& shader,
           const std::vector<Input>& inputs)
    : mWidth(size.x)
    , mHeight(size.y)
{
    vk::ShaderModule shaderModule = device.GetShaderModule(shader);

    DescriptorSetLayoutBuilder layoutBuilder;
    for (int i = 0; i < inputs.size(); i++)
    {
        layoutBuilder.Binding(i, inputs[i].Type, vk::ShaderStageFlagBits::eCompute, 1);
    }

    auto layout = layoutBuilder.Create(device);
    mDescriptor = MakeDescriptorSet(device, layout);

    DescriptorSetUpdater updater(*mDescriptor);
    for (int i = 0; i < inputs.size(); i++)
    {
        updater.WriteBuffers(i, 0, inputs[i].Type).Buffer(inputs[i].Buffer);
    }
    updater.Update(device.Handle());

    mLayout = PipelineLayoutBuilder()
            .DescriptorSetLayout(layout)
            .PushConstantRange({vk::ShaderStageFlagBits::eCompute, 0, 8})
            .Create(device.Handle());

    mPipeline = MakeComputePipeline(device.Handle(), shaderModule, *mLayout);
}

void Work::Dispatch(vk::CommandBuffer commandBuffer)
{
    commandBuffer.pushConstants(*mLayout, vk::ShaderStageFlagBits::eCompute, 0, 4, &mWidth);
    commandBuffer.pushConstants(*mLayout, vk::ShaderStageFlagBits::eCompute, 4, 4, &mHeight);
    commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eCompute, *mLayout, 0, {*mDescriptor}, {});
    commandBuffer.bindPipeline(vk::PipelineBindPoint::eCompute, *mPipeline);
    // TODO correct work group size given local size and work size
    commandBuffer.dispatch(1, 1, 1);
}

}}
