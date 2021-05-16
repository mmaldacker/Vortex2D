//
//  PrefixScan.h
//  Vortex
//

#pragma once

#include <Vortex2D/Renderer/CommandBuffer.h>
#include <Vortex2D/Renderer/Work.h>

namespace Vortex
{
namespace Fluid
{
/**
 * @brief The prefix sum operator.
 * @code{.cpp}
void PrefixSym(int input[], int n, int output[])
{
    output[0] = input[0];

    for (int i = 1; i < n; i++)
        output[i] = output[i-1] + input[i];
}
   @endcode
 */
class PrefixScan
{
public:
  /**
   * @brief A prefix scan object bound with input/output buffers, ready to be
   * dispatched.
   */
  class Bound
  {
  public:
    VORTEX2D_API void Record(vk::CommandBuffer commandBuffer);

    friend class PrefixScan;

  private:
    Bound(const std::vector<Renderer::CommandBuffer::CommandFn>& bufferBarriers,
          std::vector<Renderer::Work::Bound>&& bounds);

    std::vector<Renderer::CommandBuffer::CommandFn> mBufferBarriers;
    std::vector<Renderer::Work::Bound> mBounds;
  };

  VORTEX2D_API PrefixScan(const Renderer::Device& device, const glm::ivec2& size);

  VORTEX2D_API Bound Bind(Renderer::GenericBuffer& input,
                          Renderer::GenericBuffer& output,
                          Renderer::GenericBuffer& dispatchParams);

private:
  void BindRecursive(std::vector<Renderer::CommandBuffer::CommandFn>& bufferBarriers,
                     std::vector<Renderer::Work::Bound>& bound,
                     Renderer::GenericBuffer& input,
                     Renderer::GenericBuffer& output,
                     Renderer::GenericBuffer& dispatchParams,
                     Renderer::ComputeSize computeSize,
                     std::size_t level);

  int mSize;
  Renderer::Work mAddWork;
  Renderer::Work mPreScanWork;
  Renderer::Work mPreScanStoreSumWork;

  std::vector<Renderer::Buffer<int>> mPartialSums;
};

}  // namespace Fluid
}  // namespace Vortex
