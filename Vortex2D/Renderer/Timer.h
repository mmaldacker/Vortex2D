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
    Timer(const Device& device);

    void Start();
    void Stop();

    uint64_t GetElapsedNs();

private:
    const Device& mDevice;
    CommandBuffer mStart;
    CommandBuffer mStop;
    vk::UniqueQueryPool mPool;
};

}}

#endif
