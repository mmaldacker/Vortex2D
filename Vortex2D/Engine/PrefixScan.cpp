//
//  PrefixScan.cpp
//  Vortex2D
//

#include "PrefixScan.h"

#include "vortex2d_generated_spirv.h"

namespace Vortex2D
{
namespace Fluid
{
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
  computeSize.WorkSize =
      glm::ceil(glm::vec2(size, 1.0f) / (glm::vec2(localSize, 1.0f) * glm::vec2(2.0f, 1.0f)));

  return computeSize;
}
}  // namespace

PrefixScan::PrefixScan(const Renderer::Device& device, const glm::ivec2& size)
    : mSize(size.x * size.y)
    , mAddWork(device, Renderer::ComputeSize::Default1D(), SPIRV::PreScanAdd_comp)
    , mPreScanWork(device, Renderer::ComputeSize::Default1D(), SPIRV::PreScan_comp)
    , mPreScanStoreSumWork(device, Renderer::ComputeSize::Default1D(), SPIRV::PreScanStoreSum_comp)
{
  auto localSize = Renderer::ComputeSize::GetLocalSize1D();
  int workGroupSize = mSize;

  while ((workGroupSize = GetWorkGroupSize(workGroupSize, localSize)) > 1)
  {
    mPartialSums.emplace_back(device, workGroupSize);
  }

  assert(workGroupSize);
}

void PrefixScan::BindRecursive(std::vector<Renderer::CommandBuffer::CommandFn>& bufferBarriers,
                               std::vector<Renderer::Work::Bound>& bound,
                               Renderer::GenericBuffer& input,
                               Renderer::GenericBuffer& output,
                               Renderer::GenericBuffer& dispatchParams,
                               Renderer::ComputeSize computeSize,
                               std::size_t level)
{
  if (computeSize.WorkSize.x > 1)
  {
    assert(level < mPartialSums.size());
    auto& partialSums = mPartialSums[level];

    bound.emplace_back(mPreScanStoreSumWork.Bind(computeSize, {input, output, partialSums}));

    vk::Buffer outputBuffer = output.Handle();
    vk::Buffer partialSumsBuffer = partialSums.Handle();
    bufferBarriers.emplace_back([=](vk::CommandBuffer commandBuffer) {
      Renderer::BufferBarrier(outputBuffer,
                              commandBuffer,
                              vk::AccessFlagBits::eShaderWrite,
                              vk::AccessFlagBits::eShaderRead);
      Renderer::BufferBarrier(partialSumsBuffer,
                              commandBuffer,
                              vk::AccessFlagBits::eShaderWrite,
                              vk::AccessFlagBits::eShaderRead);
    });

    BindRecursive(bufferBarriers,
                  bound,
                  partialSums,
                  partialSums,
                  dispatchParams,
                  MakeComputeSize(computeSize.WorkSize.x),
                  level + 1);

    bound.emplace_back(mAddWork.Bind(computeSize, {partialSums, output}));
    bufferBarriers.emplace_back([=](vk::CommandBuffer commandBuffer) {
      Renderer::BufferBarrier(outputBuffer,
                              commandBuffer,
                              vk::AccessFlagBits::eShaderWrite,
                              vk::AccessFlagBits::eShaderRead);
    });
  }
  else
  {
    bound.emplace_back(mPreScanWork.Bind(computeSize, {input, output, dispatchParams}));
    vk::Buffer outputBuffer = output.Handle();
    vk::Buffer dispatchBuffer = dispatchParams.Handle();
    bufferBarriers.emplace_back([=](vk::CommandBuffer commandBuffer) {
      Renderer::BufferBarrier(outputBuffer,
                              commandBuffer,
                              vk::AccessFlagBits::eShaderWrite,
                              vk::AccessFlagBits::eShaderRead);
      Renderer::BufferBarrier(dispatchBuffer,
                              commandBuffer,
                              vk::AccessFlagBits::eShaderWrite,
                              vk::AccessFlagBits::eShaderRead);
    });
  }
}

PrefixScan::Bound PrefixScan::Bind(Renderer::GenericBuffer& input,
                                   Renderer::GenericBuffer& output,
                                   Renderer::GenericBuffer& dispatchParams)
{
  std::vector<Renderer::CommandBuffer::CommandFn> bufferBarriers;
  std::vector<Renderer::Work::Bound> bounds;

  BindRecursive(bufferBarriers, bounds, input, output, dispatchParams, MakeComputeSize(mSize), 0);

  return Bound(bufferBarriers, std::move(bounds));
}

PrefixScan::Bound::Bound(const std::vector<Renderer::CommandBuffer::CommandFn>& bufferBarriers,
                         std::vector<Renderer::Work::Bound>&& bounds)
    : mBufferBarriers(bufferBarriers), mBounds(std::move(bounds))
{
}

void PrefixScan::Bound::Record(vk::CommandBuffer commandBuffer)
{
  for (std::size_t i = 0; i < mBounds.size(); i++)
  {
    mBounds[i].Record(commandBuffer);
    mBufferBarriers[i](commandBuffer);
  }
}

}  // namespace Fluid
}  // namespace Vortex2D
