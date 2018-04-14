//
//  Timer.h
//  Vortex2D
//

#ifndef Vortex2D_Timer_h
#define Vortex2D_Timer_h

#include <Vortex2D/Renderer/CommandBuffer.h>
#include <Vortex2D/Renderer/Buffer.h>

namespace Vortex2D { namespace Renderer {

/**
 * @brief Calculates the ellapsed time on the GPU.
 */
class Timer
{
public:
    VORTEX2D_API Timer(const Device& device);

    /**
     * @brief Start the timer after the current last command buffer
     */
    VORTEX2D_API void Start();


    /**
     * @brief Stop the timer after the current last command buffer
     */
    VORTEX2D_API void Stop();

    /**
     * @brief Get the elapsed time between the Start and Stop calls.
     * Blocking function which will download the timestamps from the GPU.
     * @return timestamp in nanoseconds.
     */
    VORTEX2D_API uint64_t GetElapsedNs();

private:
    const Device& mDevice;
    CommandBuffer mStart;
    CommandBuffer mStop;
    vk::UniqueQueryPool mPool;
};

}}

#endif
