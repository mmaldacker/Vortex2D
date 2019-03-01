//
//  Cfl.cpp
//  Vortex2D
//

#include "Cfl.h"

#include "vortex2d_generated_spirv.h"

namespace Vortex2D
{
namespace Fluid
{
Cfl::Cfl(const Renderer::Device& device, const glm::ivec2& size, Velocity& velocity)
    : mSize(size)
    , mVelocity(velocity)
    , mVelocityMaxWork(device, size, SPIRV::VelocityMax_comp)
    , mVelocityMax(device, size.x * size.y)
    , mCfl(device, 1, VMA_MEMORY_USAGE_GPU_TO_CPU)
    , mVelocityMaxCmd(device, true)
    , mReduceVelocityMax(device, size)
{
  mVelocityMaxBound = mVelocityMaxWork.Bind({mVelocity, mVelocityMax});
  mReduceVelocityMaxBound = mReduceVelocityMax.Bind(mVelocityMax, mCfl);
  mVelocityMaxCmd.Record([&](vk::CommandBuffer commandBuffer) {
    commandBuffer.debugMarkerBeginEXT({"CFL", {{0.65f, 0.97f, 0.78f, 1.0f}}});

    mVelocityMaxBound.Record(commandBuffer);
    mReduceVelocityMaxBound.Record(commandBuffer);

    commandBuffer.debugMarkerEndEXT();
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
}  // namespace Vortex2D
