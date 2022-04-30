//
//  Timer.cpp
//  Vortex
//

#include <Vortex/Renderer/Timer.h>

#include <Vortex/Renderer/CommandBuffer.h>
#include <Vortex/Renderer/Device.h>

#include "Device.h"

namespace Vortex
{
namespace Renderer
{
namespace
{
uint64_t GetMask(uint32_t validBits)
{
  if (validBits == 64)
  {
    return static_cast<uint64_t>(-1);
  }
  else
  {
    return (1 << validBits) - 1;
  }
}
}  // namespace

struct Timer::Impl
{
  Impl(Device& device) : mDevice(static_cast<VulkanDevice&>(device)), mStart(device), mStop(device)
  {
    auto queryPoolInfo =
        vk::QueryPoolCreateInfo().setQueryType(vk::QueryType::eTimestamp).setQueryCount(2);

    mPool = mDevice.Handle().createQueryPoolUnique(queryPoolInfo);

    mStart.Record(
        [&](CommandEncoder& command)
        {
          vk::CommandBuffer cmd = reinterpret_cast<VkCommandBuffer>(command.Handle());
          cmd.resetQueryPool(*mPool, 0, 2);
          cmd.writeTimestamp(vk::PipelineStageFlagBits::eAllCommands, *mPool, 0);
        });

    mStop.Record(
        [&](CommandEncoder& command)
        {
          vk::CommandBuffer cmd = reinterpret_cast<VkCommandBuffer>(command.Handle());
          cmd.writeTimestamp(vk::PipelineStageFlagBits::eAllCommands, *mPool, 1);
        });
  }

  void Start(CommandEncoder& command)
  {
    vk::CommandBuffer cmd = reinterpret_cast<VkCommandBuffer>(command.Handle());
    cmd.resetQueryPool(*mPool, 0, 2);
    cmd.writeTimestamp(vk::PipelineStageFlagBits::eAllCommands, *mPool, 0);
  }

  void Stop(CommandEncoder& command)
  {
    vk::CommandBuffer cmd = reinterpret_cast<VkCommandBuffer>(command.Handle());
    cmd.writeTimestamp(vk::PipelineStageFlagBits::eAllCommands, *mPool, 1);
  }

  void Start() { mStart.Submit(); }

  void Stop() { mStop.Submit(); }

  void Wait()
  {
    mStart.Wait();
    mStop.Wait();
  }

  uint64_t GetElapsedNs()
  {
    uint64_t timestamps[2] = {0};
    auto result = mDevice.Handle().getQueryPoolResults(
        *mPool,
        0,
        2,
        sizeof(timestamps),
        timestamps,
        sizeof(uint64_t),
        vk::QueryResultFlagBits::eWait | vk::QueryResultFlagBits::e64);

    if (result == vk::Result::eSuccess)
    {
      int familyIndex = mDevice.GetFamilyIndex();
      auto properties = mDevice.GetPhysicalDevice().getProperties();
      assert(properties.limits.timestampComputeAndGraphics);

      uint64_t period = static_cast<uint64_t>(properties.limits.timestampPeriod);

      auto queueProperties = mDevice.GetPhysicalDevice().getQueueFamilyProperties();
      auto validBits = queueProperties[familyIndex].timestampValidBits;

      return ((timestamps[1] & GetMask(validBits)) - (timestamps[0] & GetMask(validBits))) * period;
    }
    else
    {
      return static_cast<uint64_t>(-1);
    }
  }

  VulkanDevice& mDevice;
  CommandBuffer mStart;
  CommandBuffer mStop;
  vk::UniqueQueryPool mPool;
};

Timer::Timer(Device& device) : mImpl(std::make_unique<Impl>(device)) {}

Timer::~Timer() {}

void Timer::Start(CommandEncoder& command)
{
  mImpl->Start(command);
}

void Timer::Stop(CommandEncoder& command)
{
  mImpl->Stop(command);
}

void Timer::Start()
{
  mImpl->Start();
}

void Timer::Stop()
{
  mImpl->Stop();
}

void Timer::Wait()
{
  mImpl->Wait();
}

uint64_t Timer::GetElapsedNs()
{
  return mImpl->GetElapsedNs();
}

}  // namespace Renderer
}  // namespace Vortex
