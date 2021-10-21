//
//  Reduce.cpp
//  Vortex
//

#include "Reduce.h"

#include <Vortex/Renderer/DescriptorSet.h>
#include <Vortex/Renderer/Work.h>

#include "vortex_generated_spirv.h"

namespace Vortex
{
namespace Fluid
{
namespace
{
Renderer::ComputeSize MakeComputeSize(int size)
{
  Renderer::ComputeSize computeSize(Renderer::ComputeSize::Default1D());

  auto localSize = Renderer::ComputeSize::GetLocalSize1D();
  computeSize.DomainSize = glm::ivec2(size, 1);
  computeSize.LocalSize = glm::ivec2(localSize, 1);
  computeSize.WorkSize = glm::ceil(glm::vec2(size, 1.0f) / glm::vec2(localSize * 2, 1.0f));

  return computeSize;
}
}  // namespace

Reduce::Reduce(Renderer::Device& device,
               const Renderer::SpirvBinary& spirv,
               int size,
               std::size_t typeSize)
    : mSize(size), mReduce(device, Renderer::ComputeSize::Default1D(), spirv)
{
  auto computeSize = MakeComputeSize(mSize);
  while (computeSize.WorkSize.x > 1)
  {
    mBuffers.emplace_back(device,
                          vk::BufferUsageFlagBits::eStorageBuffer,
                          VMA_MEMORY_USAGE_GPU_ONLY,
                          typeSize * computeSize.WorkSize.x);

    computeSize = MakeComputeSize(computeSize.WorkSize.x);
  }

  assert(computeSize.WorkSize.x == 1);
}

Reduce::Bound Reduce::Bind(Renderer::GenericBuffer& input, Renderer::GenericBuffer& output)
{
  std::vector<Renderer::GenericBuffer*> buffers;
  buffers.push_back(&input);
  for (auto& buffer : mBuffers)
  {
    buffers.push_back(&buffer);
  }
  buffers.push_back(&output);

  std::vector<Renderer::CommandBuffer::CommandFn> bufferBarriers;
  std::vector<Renderer::Work::Bound> bounds;

  auto computeSize = MakeComputeSize(mSize);
  for (std::size_t i = 0; i < buffers.size() - 1; i++)
  {
    bounds.emplace_back(mReduce.Bind(computeSize, {*buffers[i], *buffers[i + 1]}));
    computeSize = MakeComputeSize(computeSize.WorkSize.x);

    vk::Buffer buffer = buffers[i + 1]->Handle();
    bufferBarriers.emplace_back(
        [=](vk::CommandBuffer commandBuffer)
        {
          Renderer::BufferBarrier(buffer,
                                  commandBuffer,
                                  vk::AccessFlagBits::eShaderWrite,
                                  vk::AccessFlagBits::eShaderRead);
        });
  }

  return Bound(mSize, bufferBarriers, std::move(bounds));
}

Reduce::Bound::Bound(int size,
                     const std::vector<Renderer::CommandBuffer::CommandFn>& bufferBarriers,
                     std::vector<Renderer::Work::Bound>&& bounds)
    : mSize(size), mBufferBarriers(bufferBarriers), mBounds(std::move(bounds))
{
}

void Reduce::Bound::Record(vk::CommandBuffer commandBuffer)
{
  int localSize = 2 * Renderer::ComputeSize::GetLocalSize1D();
  int workGroupSize = mSize;

  for (std::size_t i = 0; i < mBounds.size(); i++)
  {
    mBounds[i].Record(commandBuffer);
    mBufferBarriers[i](commandBuffer);

    workGroupSize = (workGroupSize + localSize - 1) / localSize;
  }
}

ReduceSum::ReduceSum(Renderer::Device& device, int size)
    : Reduce(device, SPIRV::Sum_comp, size, sizeof(float))
{
}

// TODO should merge with struct in Rigidbody
struct J
{
  alignas(8) glm::vec2 linear;
  alignas(4) float angular;
};

ReduceJ::ReduceJ(Renderer::Device& device, int size)
    : Reduce(device, SPIRV::SumJ_comp, size, sizeof(J))
{
}

ReduceMax::ReduceMax(Renderer::Device& device, int size)
    : Reduce(device, SPIRV::Max_comp, size, sizeof(float))
{
}

}  // namespace Fluid
}  // namespace Vortex
