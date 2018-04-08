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

    VORTEX2D_API void Start();
    VORTEX2D_API void Stop();

    VORTEX2D_API uint64_t GetElapsedNs();

private:
    const Device& mDevice;
    CommandBuffer mStart;
    CommandBuffer mStop;
    vk::UniqueQueryPool mPool;
};

}}

#endif
