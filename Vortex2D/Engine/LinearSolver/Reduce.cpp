//
//  Reduce.cpp
//  Vortex2D
//

#include "Reduce.h"

#include <Vortex2D/Renderer/DescriptorSet.h>

namespace Vortex2D { namespace Fluid {

namespace
{
    int GetWorkGroupSize(int n, int localSize)
    {
        return (n + (localSize * 2 - 1)) / (localSize * 2);
    }

    int GetBufferSize(const glm::vec2& size)
    {
        int localSize = Renderer::GetLocalSize(size.x * size.y, 1).x;
        return GetWorkGroupSize(size.x * size.y, localSize);
    }
}

Reduce::Reduce(const Renderer::Device& device,
               const std::string& fileName,
               const glm::vec2& size,
               Renderer::Buffer& input,
               Renderer::Buffer& output)
    : mCommandBuffer(device)
    , mSize(size.x * size.y)
    , mReduce(device,
              vk::BufferUsageFlagBits::eStorageBuffer,
              true,
              sizeof(float) * GetBufferSize(size))
{
    static vk::DescriptorSetLayout descriptorLayout = Renderer::DescriptorSetLayoutBuilder()
            .Binding(0, vk::DescriptorType::eStorageBuffer, vk::ShaderStageFlagBits::eCompute, 1)
            .Binding(1, vk::DescriptorType::eStorageBuffer, vk::ShaderStageFlagBits::eCompute, 1)
            .Create(device);

    mDescriptorSet0 = Renderer::MakeDescriptorSet(device, descriptorLayout);
    mDescriptorSet1 = Renderer::MakeDescriptorSet(device, descriptorLayout);

    Renderer::DescriptorSetUpdater(*mDescriptorSet0)
            .WriteBuffers(0, 0, vk::DescriptorType::eStorageBuffer).Buffer(input)
            .WriteBuffers(1, 0, vk::DescriptorType::eStorageBuffer).Buffer(mReduce)
            .Update(device.Handle());

    Renderer::DescriptorSetUpdater(*mDescriptorSet1)
            .WriteBuffers(0, 0, vk::DescriptorType::eStorageBuffer).Buffer(mReduce)
            .WriteBuffers(1, 0, vk::DescriptorType::eStorageBuffer).Buffer(output)
            .Update(device.Handle());

    mPipelineLayout = Renderer::PipelineLayoutBuilder()
            .DescriptorSetLayout(descriptorLayout)
            .PushConstantRange({vk::ShaderStageFlagBits::eCompute, 0, 4})
            .Create(device.Handle());

    vk::ShaderModule sumShader = device.GetShaderModule(fileName);

    int n = size.x * size.y;
    auto localSize = Renderer::GetLocalSize(n, 1);
    int workGroupSize0 = GetWorkGroupSize(n, localSize.x);
    int workGroupSize1 = GetWorkGroupSize(workGroupSize0, localSize.x);

    // TODO fix and do recursive loop
    assert(workGroupSize1 == 1);

    std::array<int, 2> constants = {{localSize.x, localSize.x}};

    std::vector<vk::SpecializationMapEntry> mapEntries = {{1, 0, 4}, {2, 0, 4}};

    auto specialisationConst = vk::SpecializationInfo()
            .setMapEntryCount(mapEntries.size())
            .setPMapEntries(mapEntries.data())
            .setDataSize(8)
            .setPData(constants.data());

    mPipeline = Renderer::MakeComputePipeline(device.Handle(), sumShader, *mPipelineLayout, specialisationConst);

    mCommandBuffer.Record([&](vk::CommandBuffer commandBuffer)
    {
        input.Barrier(commandBuffer, vk::AccessFlagBits::eShaderRead);
        mReduce.Barrier(commandBuffer, vk::AccessFlagBits::eShaderWrite);

        commandBuffer.bindPipeline(vk::PipelineBindPoint::eCompute, *mPipeline);

        commandBuffer.pushConstants(*mPipelineLayout, vk::ShaderStageFlagBits::eCompute, 0, 4, &mSize);
        commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eCompute, *mPipelineLayout, 0, {*mDescriptorSet0}, {});
        commandBuffer.dispatch(workGroupSize0, 1, 1);

        mReduce.Barrier(commandBuffer, vk::AccessFlagBits::eShaderRead);
        output.Barrier(commandBuffer, vk::AccessFlagBits::eShaderWrite);

        commandBuffer.pushConstants(*mPipelineLayout, vk::ShaderStageFlagBits::eCompute, 0, 4, &workGroupSize0);
        commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eCompute, *mPipelineLayout, 0, {*mDescriptorSet1}, {});
        commandBuffer.dispatch(workGroupSize1, 1, 1);
    });
}

void Reduce::Submit()
{
    mCommandBuffer.Submit();
}

ReduceSum::ReduceSum(const Renderer::Device& device,
                     const glm::vec2& size,
                     Renderer::Buffer& input,
                     Renderer::Buffer& output)
    : Reduce(device, "../Vortex2D/Sum.comp.spv", size, input, output)
{

}

ReduceMax::ReduceMax(const Renderer::Device& device,
                     const glm::vec2& size,
                     Renderer::Buffer& input,
                     Renderer::Buffer& output)
    : Reduce(device, "../Vortex2D/Max.comp.spv", size, input, output)
{

}

}}
