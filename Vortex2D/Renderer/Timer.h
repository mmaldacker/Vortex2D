//
//  Timer.h
//  Vortex
//

#pragma once

#include <Vortex2D/Renderer/Buffer.h>
#include <Vortex2D/Renderer/CommandBuffer.h>

namespace Vortex
{
namespace Renderer
{
/**
 * @brief Calculates the ellapsed time on the GPU.
 */
class Timer
{
public:
  VORTEX_API Timer(const Device& device);

  /**
   * @brief Start the timer after the current last command buffer
   * @param commandBuffer command buffer to write timestamp
   */
  VORTEX_API void Start(vk::CommandBuffer commandBuffer);

  /**
   * @brief Start the timer after the current last command buffer
   * @param commandBuffer command buffer to write timestamp
   */
  VORTEX_API void Stop(vk::CommandBuffer commandBuffer);

  /**
   * @brief Start the timer after the current last command buffer
   */
  VORTEX_API void Start();

  /**
   * @brief Stop the timer after the current last command buffer
   */
  VORTEX_API void Stop();

  /**
   * @brief Wait for @ref Start and @ref Stop to finish before retrieving the
   * results
   */
  VORTEX_API void Wait();

  /**
   * @brief Get the elapsed time between the Start and Stop calls.
   * Blocking function which will download the timestamps from the GPU.
   * @return timestamp in nanoseconds.
   */
  VORTEX_API uint64_t GetElapsedNs();

private:
  const Device& mDevice;
  CommandBuffer mStart;
  CommandBuffer mStop;
  vk::UniqueQueryPool mPool;
};

}  // namespace Renderer
}  // namespace Vortex
