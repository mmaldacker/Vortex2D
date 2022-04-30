//
//  Cfl.cpp
//  Vortex
//

#include "Cfl.h"

#include "vortex_generated_spirv.h"

namespace Vortex
{
namespace Fluid
{
Cfl::Cfl(Renderer::Device& device, const glm::ivec2& size, Velocity& velocity)
    : mDevice(device)
    , mSize(size)
    , mVelocity(velocity)
    , mVelocityMaxWork(device, Renderer::ComputeSize{size}, SPIRV::VelocityMax_comp)
    , mVelocityMax(device, size.x * size.y)
    , mCfl(device, 1, Renderer::MemoryUsage::GpuToCpu)
    , mVelocityMaxCmd(device, true)
    , mReduceVelocityMax(device, size.x * size.y)
{
  mVelocityMaxBound = mVelocityMaxWork.Bind({mVelocity, mVelocityMax});
  mReduceVelocityMaxBound = mReduceVelocityMax.Bind(mVelocityMax, mCfl);
  mVelocityMaxCmd.Record(
      [&](Renderer::CommandEncoder& command)
      {
        command.DebugMarkerBegin("CFL", {0.65f, 0.97f, 0.78f, 1.0f});

        mVelocityMaxBound.Record(command);
        mVelocityMax.Barrier(command, Renderer::Access::Write, Renderer::Access::Read);
        mReduceVelocityMaxBound.Record(command);

        command.DebugMarkerEnd();
      });
}

void Cfl::Compute()
{
  mVelocityMaxCmd.Submit();
}

float Cfl::Get()
{
  mVelocityMaxCmd.Wait();

  float cfl;
  Renderer::CopyTo(mCfl, cfl);

  return 1.0f / (cfl * mSize.x);
}

}  // namespace Fluid
}  // namespace Vortex
