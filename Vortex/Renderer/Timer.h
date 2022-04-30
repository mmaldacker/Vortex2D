//
//  Timer.h
//  Vortex
//

#pragma once

#include <Vortex/Renderer/Buffer.h>
#include <Vortex/Renderer/CommandBuffer.h>

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
  VORTEX_API Timer(Device& device);
  VORTEX_API ~Timer();

  /**
   * @brief Start the timer after the current last command buffer
   * @param commandBuffer command buffer to write timestamp
   */
  VORTEX_API void Start(CommandEncoder& command);

  /**
   * @brief Start the timer after the current last command buffer
   * @param commandBuffer command buffer to write timestamp
   */
  VORTEX_API void Stop(CommandEncoder& command);

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
  struct Impl;
  std::unique_ptr<Impl> mImpl;
};

}  // namespace Renderer
}  // namespace Vortex
