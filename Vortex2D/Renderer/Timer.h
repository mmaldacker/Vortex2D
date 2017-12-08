//
//  Timer.h
//  Vortex2D
//

#ifndef Vortex2D_Timer_h
#define Vortex2D_Timer_h

#include <Vortex2D/Renderer/CommandBuffer.h>
#include <Vortex2D/Renderer/Buffer.h>

namespace Vortex2D { namespace Renderer {

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

class Statistics
{
public:
    using Timestamps = std::vector<std::pair<std::string, uint64_t>>;

    Statistics(const Device& device);

    void Start(vk::CommandBuffer commandBuffer);
    void Tick(vk::CommandBuffer commandBuffer, const std::string& name);
    void End(vk::CommandBuffer commandBuffer, const std::string& name);

    Timestamps GetTimestamps();
private:
    const Device& mDevice;
    vk::UniqueQueryPool mPool;
    int mIndex;
    Buffer<uint64_t> mResults;

    std::vector<std::string> mTimestampNames;
};

}}

#endif
