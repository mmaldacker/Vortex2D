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

Renderer::ComputeSize MakeComputeSize(int size)
{
    Renderer::ComputeSize computeSize(Renderer::ComputeSize::Default1D());

    auto localSize = Renderer::ComputeSize::GetLocalSize1D();
    computeSize.DomainSize = glm::ivec2(size, 1);
    computeSize.LocalSize = glm::ivec2(localSize, 1);
    computeSize.WorkSize = glm::ceil(glm::vec2(size, 1.0f) / (glm::vec2(localSize, 1.0f) * glm::vec2(2.0f, 1.0f)));

    return computeSize;
}
}

Reduce::Reduce(const Renderer::Device& device,
               const std::string& fileName,
               const glm::ivec2& size)
    : mSize(size.x*size.y)
    , mReduce(device, Renderer::ComputeSize::Default1D(), fileName)
{
    auto localSize = Renderer::ComputeSize::GetLocalSize1D();
    int workGroupSize = mSize;

    while ((workGroupSize = GetWorkGroupSize(workGroupSize, localSize)) > 1)
    {
        mBuffers.emplace_back(device, workGroupSize);
    }

    assert(workGroupSize);
}

Reduce::Bound Reduce::Bind(Renderer::GenericBuffer& input,
                           Renderer::GenericBuffer& output)
{
    std::vector<Renderer::GenericBuffer*> buffers;
    buffers.push_back(&input);
    for (auto& buffer: mBuffers)
    {
        buffers.push_back(&buffer);
    }
    buffers.push_back(&output);

    std::vector<Renderer::CommandBuffer::CommandFn> bufferBarriers;
    std::vector<Renderer::Work::Bound> bounds;

    auto computeSize = MakeComputeSize(mSize);
    for (int i = 0; i < buffers.size() - 1; i++)
    {
        bounds.emplace_back(mReduce.Bind(computeSize, {*buffers[i], *buffers[i+1]}));
        computeSize = MakeComputeSize(computeSize.WorkSize.x);

        vk::Buffer buffer = buffers[i+1]->Handle();
        bufferBarriers.emplace_back([=](vk::CommandBuffer commandBuffer)
        {
            Renderer::BufferBarrier(buffer, commandBuffer, vk::AccessFlagBits::eShaderWrite, vk::AccessFlagBits::eShaderRead);
        });
    }

    return Bound(bufferBarriers, std::move(bounds));
}

Reduce::Bound::Bound(const std::vector<Renderer::CommandBuffer::CommandFn>& bufferBarriers,
                     std::vector<Renderer::Work::Bound>&& bounds)
    : mBufferBarriers(bufferBarriers)
    , mBounds(std::move(bounds))
{
}

void Reduce::Bound::Record(vk::CommandBuffer commandBuffer)
{
    for (int i = 0; i < mBounds.size(); i++)
    {
        mBounds[i].Record(commandBuffer);
        mBufferBarriers[i](commandBuffer);
    }
}

ReduceSum::ReduceSum(const Renderer::Device& device,
                     const glm::ivec2& size)
    : Reduce(device, "Sum.comp.spv", size)
{

}

ReduceMax::ReduceMax(const Renderer::Device& device,
                     const glm::ivec2& size)
    : Reduce(device, "Max.comp.spv", size)
{

}

}}
