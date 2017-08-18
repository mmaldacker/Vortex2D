//
//  Timer.cpp
//  Vortex2D
//

#include "Timer.h"

#include <Vortex2D/Renderer/CommandBuffer.h>

namespace Vortex2D { namespace Renderer {

namespace
{
    const int queryCount = 128;
}

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

    // TODO set mask using validBits
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

Statistics::Statistics(const Device& device)
    : mDevice(device)
    , mIndex(0)
    , mResults(device, vk::BufferUsageFlagBits::eStorageBuffer, false, sizeof(uint64_t) * queryCount)
    , mOutput(device, vk::BufferUsageFlagBits::eStorageBuffer, true, sizeof(uint64_t) * queryCount)
{
    auto queryPoolInfo = vk::QueryPoolCreateInfo()
            .setQueryType(vk::QueryType::eTimestamp)
            .setQueryCount(queryCount);

    mPool = device.Handle().createQueryPoolUnique(queryPoolInfo);
}

void Statistics::Start(vk::CommandBuffer commandBuffer)
{
    mIndex = 0;
    mTimestampNames.clear();
    commandBuffer.writeTimestamp(vk::PipelineStageFlagBits::eAllCommands, *mPool, 0);
}

void Statistics::Tick(vk::CommandBuffer commandBuffer, const std::string& name)
{
    commandBuffer.writeTimestamp(vk::PipelineStageFlagBits::eAllCommands, *mPool, ++mIndex);
    mTimestampNames.emplace_back(name);
}

void Statistics::End(vk::CommandBuffer commandBuffer, const std::string& name)
{
    commandBuffer.writeTimestamp(vk::PipelineStageFlagBits::eAllCommands, *mPool, ++mIndex);
    commandBuffer.copyQueryPoolResults(*mPool, 0, mIndex + 1, mResults, 0, sizeof(uint64_t), vk::QueryResultFlagBits::eWait | vk::QueryResultFlagBits::e64);
    commandBuffer.resetQueryPool(*mPool, 0, mIndex + 1);
    mTimestampNames.emplace_back(name);
}

Statistics::Timestamps Statistics::GetTimestamps()
{
    ExecuteCommand(mDevice, [&](vk::CommandBuffer commandBuffer)
    {
        mOutput.CopyFrom(commandBuffer, mResults);
    });

    std::vector<uint64_t> timestamps(queryCount);
    mOutput.CopyTo(timestamps);

    Timestamps result;
    for (int i = 0; i < mIndex; i++)
    {
        uint64_t diff = timestamps[i + 1] - timestamps[i];
        result.emplace_back(mTimestampNames[i], diff);
    }

    return result;
}

}}
