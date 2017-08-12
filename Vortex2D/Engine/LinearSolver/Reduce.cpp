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

    int GetBufferSize(const glm::ivec2& size)
    {
        int localSize = Renderer::GetLocalSize(size.x * size.y, 1).x;
        return GetWorkGroupSize(size.x * size.y, localSize);
    }
}

Reduce::Reduce(const Renderer::Device& device,
               const std::string& fileName,
               const glm::ivec2& size)
    : mDevice(device)
    , mSize(size.x * size.y)
    , mReduce(device,
              vk::BufferUsageFlagBits::eStorageBuffer,
              true,
              sizeof(float) * GetBufferSize(size))
{
    mDescriptorLayout = Renderer::DescriptorSetLayoutBuilder()
            .Binding(0, vk::DescriptorType::eStorageBuffer, vk::ShaderStageFlagBits::eCompute, 1)
            .Binding(1, vk::DescriptorType::eStorageBuffer, vk::ShaderStageFlagBits::eCompute, 1)
            .Create(device);

    vk::ShaderModule sumShader = device.GetShaderModule(fileName);

    int n = size.x * size.y;
    auto localSize = Renderer::GetLocalSize(n, 1);
    int workGroupSize0 = GetWorkGroupSize(n, localSize.x);
    int workGroupSize1 = GetWorkGroupSize(workGroupSize0, localSize.x);

    // TODO fix and do recursive loop
    assert(workGroupSize1 == 1);

    mPipelineLayout = Renderer::PipelineLayoutBuilder()
            .DescriptorSetLayout(mDescriptorLayout)
            .PushConstantRange({vk::ShaderStageFlagBits::eCompute, 0, 4})
            .Create(device.Handle());

    std::array<uint32_t, 2> constants = {{localSize.x, localSize.x}};

    std::vector<vk::SpecializationMapEntry> mapEntries = {{1, 0, 4}, {2, 0, 4}};

    auto specialisationConst = vk::SpecializationInfo()
            .setMapEntryCount((uint32_t)mapEntries.size())
            .setPMapEntries(mapEntries.data())
            .setDataSize(8)
            .setPData(constants.data());

    mPipeline = Renderer::MakeComputePipeline(device.Handle(), sumShader, *mPipelineLayout, specialisationConst);
}

Reduce::Bound Reduce::Bind(Renderer::Buffer& input, Renderer::Buffer& output)
{
    std::vector<vk::UniqueDescriptorSet> descriptorSets;
    descriptorSets.emplace_back(Renderer::MakeDescriptorSet(mDevice, mDescriptorLayout));
    descriptorSets.emplace_back(Renderer::MakeDescriptorSet(mDevice, mDescriptorLayout));

    Renderer::DescriptorSetUpdater(*descriptorSets[0])
            .WriteBuffers(0, 0, vk::DescriptorType::eStorageBuffer).Buffer(input)
            .WriteBuffers(1, 0, vk::DescriptorType::eStorageBuffer).Buffer(mReduce)
            .Update(mDevice.Handle());

    Renderer::DescriptorSetUpdater(*descriptorSets[1])
            .WriteBuffers(0, 0, vk::DescriptorType::eStorageBuffer).Buffer(mReduce)
            .WriteBuffers(1, 0, vk::DescriptorType::eStorageBuffer).Buffer(output)
            .Update(mDevice.Handle());

    std::vector<Renderer::Buffer*> buffers = {&mReduce, &output};
    return Bound(mSize, *mPipelineLayout, *mPipeline, buffers, std::move(descriptorSets));
}

Reduce::Bound::Bound(int size,
                     vk::PipelineLayout layout,
                     vk::Pipeline pipeline,
                     const std::vector<Renderer::Buffer*>& buffers,
                     std::vector<vk::UniqueDescriptorSet>&& descriptorSets)
    : mSize(size)
    , mLayout(layout)
    , mPipeline(pipeline)
    , mBuffers(buffers)
    , mDescriptorSets(std::move(descriptorSets))
{
}

void Reduce::Bound::Record(vk::CommandBuffer commandBuffer)
{
    auto localSize = Renderer::GetLocalSize(mSize, 1);
    int workGroupSize0 = GetWorkGroupSize(mSize, localSize.x);
    int workGroupSize1 = GetWorkGroupSize(workGroupSize0, localSize.x);

    // TODO fix and do recursive loop
    assert(workGroupSize1 == 1);

    commandBuffer.bindPipeline(vk::PipelineBindPoint::eCompute, mPipeline);

    commandBuffer.pushConstants(mLayout, vk::ShaderStageFlagBits::eCompute, 0, 4, &mSize);
    commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eCompute, mLayout, 0, {*mDescriptorSets[0]}, {});
    commandBuffer.dispatch(workGroupSize0, 1, 1);

    mBuffers[0]->Barrier(commandBuffer, vk::AccessFlagBits::eShaderWrite, vk::AccessFlagBits::eShaderRead);
    mBuffers[1]->Barrier(commandBuffer, vk::AccessFlagBits::eShaderRead, vk::AccessFlagBits::eShaderWrite);

    commandBuffer.pushConstants(mLayout, vk::ShaderStageFlagBits::eCompute, 0, 4, &workGroupSize0);
    commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eCompute, mLayout, 0, {*mDescriptorSets[1]}, {});
    commandBuffer.dispatch(workGroupSize1, 1, 1);
}

ReduceSum::ReduceSum(const Renderer::Device& device,
                     const glm::ivec2& size)
    : Reduce(device, "../Vortex2D/Sum.comp.spv", size)
{

}

ReduceMax::ReduceMax(const Renderer::Device& device,
                     const glm::ivec2& size)
    : Reduce(device, "../Vortex2D/Max.comp.spv", size)
{

}

}}
