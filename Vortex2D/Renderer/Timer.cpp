//
//  Timer.cpp
//  Vortex2D
//

#include "Timer.h"

#include <Vortex2D/Renderer/CommandBuffer.h>

namespace Vortex2D { namespace Renderer {

Timer::Timer(const Device& device)
    : mDevice(device)
    , mStart(device)
    , mStop(device)
{
    auto queryPoolInfo = vk::QueryPoolCreateInfo()
            .setQueryType(vk::QueryType::eTimestamp)
            .setQueryCount(2);

    mPool = device.Handle().createQueryPoolUnique(queryPoolInfo);

    mStart.Record([&](vk::CommandBuffer commandBuffer)
    {
        commandBuffer.writeTimestamp(vk::PipelineStageFlagBits::eAllCommands, *mPool, 0);
    });

    mStop.Record([&](vk::CommandBuffer commandBuffer)
    {
        commandBuffer.writeTimestamp(vk::PipelineStageFlagBits::eAllCommands, *mPool, 1);
    });
}

void Timer::Start()
{
    mStart.Submit();
}

void Timer::Stop()
{
    mStop.Submit();
}

uint64_t Timer::GetElapsedNs()
{
    mStart.Wait();
    mStop.Wait();

    const uint64_t invalidTime = -1;

    uint64_t timestamps[2] = {0};
    auto result = mDevice.Handle().getQueryPoolResults(*mPool, 0, 2, sizeof(timestamps), timestamps, 0,
                                                       vk::QueryResultFlagBits::eWait | vk::QueryResultFlagBits::e64);

    ExecuteCommand(mDevice, [&](vk::CommandBuffer commandBuffer)
    {
       commandBuffer.resetQueryPool(*mPool, 0, 2);
    });

    if (result == vk::Result::eSuccess)
    {
        int familyIndex = mDevice.GetFamilyIndex();
        auto properties = mDevice.GetPhysicalDevice().getProperties();
        assert(properties.limits.timestampComputeAndGraphics);

        double period = properties.limits.timestampPeriod;

        auto queueProperties = mDevice.GetPhysicalDevice().getQueueFamilyProperties();
        auto validBits = queueProperties[familyIndex].timestampValidBits;

        return ((timestamps[1] & invalidTime) - (timestamps[0] & invalidTime)) * period;
    }
    else
    {
        return invalidTime;
    }
}

}}
