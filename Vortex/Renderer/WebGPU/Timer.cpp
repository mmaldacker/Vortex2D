//
//  Timer.cpp
//  Vortex2D
//

#include <Vortex/Renderer/Timer.h>

#include <Vortex/Renderer/CommandBuffer.h>
#include <Vortex/Renderer/Device.h>

#include "Device.h"

namespace Vortex
{
namespace Renderer
{
struct Timer::Impl
{
  Impl(Device& device) : mStart(device), mStop(device) {}

  void Start(CommandEncoder& command) {}

  void Stop(CommandEncoder& command) {}

  void Start() { mStart.Submit(); }

  void Stop() { mStop.Submit(); }

  void Wait()
  {
    mStart.Wait();
    mStop.Wait();
  }

  uint64_t GetElapsedNs() { return -1; }

  CommandBuffer mStart;
  CommandBuffer mStop;
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
