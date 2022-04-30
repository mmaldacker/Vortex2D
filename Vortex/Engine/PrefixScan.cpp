//
//  PrefixScan.cpp
//  Vortex
//

#include "PrefixScan.h"

#include "vortex_generated_spirv.h"

namespace Vortex
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

PrefixScan::PrefixScan(Renderer::Device& device, int size)
    : mSize(size)
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

    bufferBarriers.emplace_back(
        [&](Renderer::CommandEncoder& command)
        {
          output.Barrier(command, Renderer::Access::Write, Renderer::Access::Read);
          partialSums.Barrier(command, Renderer::Access::Write, Renderer::Access::Read);
        });

    BindRecursive(bufferBarriers,
                  bound,
                  partialSums,
                  partialSums,
                  dispatchParams,
                  MakeComputeSize(computeSize.WorkSize.x),
                  level + 1);

    bound.emplace_back(mAddWork.Bind(computeSize, {partialSums, output}));
    bufferBarriers.emplace_back(
        [&](Renderer::CommandEncoder& command)
        { output.Barrier(command, Renderer::Access::Write, Renderer::Access::Read); });
  }
  else
  {
    bound.emplace_back(mPreScanWork.Bind(computeSize, {input, output, dispatchParams}));

    bufferBarriers.emplace_back(
        [&](Renderer::CommandEncoder& command)
        {
          output.Barrier(command, Renderer::Access::Write, Renderer::Access::Read);
          dispatchParams.Barrier(command, Renderer::Access::Write, Renderer::Access::Read);
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

void PrefixScan::Bound::Record(Renderer::CommandEncoder& command)
{
  for (std::size_t i = 0; i < mBounds.size(); i++)
  {
    mBounds[i].Record(command);
    mBufferBarriers[i](command);
  }
}

}  // namespace Fluid
}  // namespace Vortex
