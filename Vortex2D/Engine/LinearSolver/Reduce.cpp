//
//  Reduce.cpp
//  Vortex2D
//

#include "Reduce.h"

#include <Vortex2D/Renderer/DescriptorSet.h>
#include <Vortex2D/Renderer/Work.h>

namespace Vortex2D { namespace Fluid {

namespace
{
    int GetWorkGroupSize(int n, int localSize)
    {
        return (n + (localSize * 2 - 1)) / (localSize * 2);
    }
}

Reduce::Reduce(const Renderer::Device& device,
               const std::string& fileName,
               const glm::ivec2& size)
    : mDevice(device)
    , mSize(size.x * size.y)
{
    mDescriptorLayout = Renderer::DescriptorSetLayoutBuilder()
            .Binding(0, vk::DescriptorType::eStorageBuffer, vk::ShaderStageFlagBits::eCompute, 1)
            .Binding(1, vk::DescriptorType::eStorageBuffer, vk::ShaderStageFlagBits::eCompute, 1)
            .Create(device);

    vk::ShaderModule sumShader = device.GetShaderModule(fileName);

    int n = size.x * size.y;
    auto localSize = Renderer::ComputeSize::GetLocalSize1D();

    int workGroupSize = n;
    while ((workGroupSize = GetWorkGroupSize(workGroupSize, localSize)) > 1)
    {
        mBuffers.emplace_back(device, vk::BufferUsageFlagBits::eStorageBuffer, false, sizeof(float) * workGroupSize);
    }

    assert(workGroupSize);

    mPipelineLayout = Renderer::PipelineLayoutBuilder()
            .DescriptorSetLayout(mDescriptorLayout)
            .PushConstantRange({vk::ShaderStageFlagBits::eCompute, 0, 4})
            .Create(device.Handle());

    std::array<uint32_t, 2> constants = {{localSize, localSize}};

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

    // set buffer pointers to be able to call barrier on it later
    std::vector<Renderer::Buffer*> buffers;
    buffers.push_back(&input);
    for (auto& buffer: mBuffers)
    {
        buffers.push_back(&buffer);
    }
    buffers.push_back(&output);

    // create and set descriptors
    for (int i = 0; i < buffers.size() - 1; i++)
    {
        descriptorSets.emplace_back(Renderer::MakeDescriptorSet(mDevice, mDescriptorLayout));

        Renderer::DescriptorSetUpdater(*descriptorSets.back())
                .WriteBuffers(0, 0, vk::DescriptorType::eStorageBuffer).Buffer(*buffers[i])
                .WriteBuffers(1, 0, vk::DescriptorType::eStorageBuffer).Buffer(*buffers[i+1])
                .Update(mDevice.Handle());
    }

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
    auto localSize = Renderer::ComputeSize::GetLocalSize1D();

    commandBuffer.bindPipeline(vk::PipelineBindPoint::eCompute, mPipeline);

    int workGroupSize = mSize;
    for (int i = 0; i < mDescriptorSets.size(); i++)
    {
        commandBuffer.pushConstants(mLayout, vk::ShaderStageFlagBits::eCompute, 0, 4, &workGroupSize);
        workGroupSize = GetWorkGroupSize(workGroupSize, localSize);

        commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eCompute, mLayout, 0, {*mDescriptorSets[i]}, {});
        commandBuffer.dispatch(workGroupSize, 1, 1);

        mBuffers[i + 1]->Barrier(commandBuffer, vk::AccessFlagBits::eShaderWrite, vk::AccessFlagBits::eShaderRead);
    }
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
