//
//  Velocity.cpp
//  Vortex
//

#include "Velocity.h"

#include "vortex_generated_spirv.h"

namespace Vortex
{
namespace Fluid
{
Velocity::Velocity(Renderer::Device& device, const glm::ivec2& size)
    : Renderer::RenderTexture(device, size.x, size.y, Renderer::Format::R32G32Sfloat)
    , mDevice(device)
    , mOutputVelocity(device, size.x, size.y, Renderer::Format::R32G32Sfloat)
    , mDVelocity(device, size.x, size.y, Renderer::Format::R32G32Sfloat)
    , mVelocityDiff(device, Renderer::ComputeSize{size}, SPIRV::VelocityDifference_comp)
    , mVelocityDiffBound(mVelocityDiff.Bind({mDVelocity, *this, mOutputVelocity}))
    , mSaveCopyCmd(device, false)
    , mVelocityDiffCmd(device, false)
{
  mSaveCopyCmd.Record([&](Renderer::CommandEncoder& command)
                      { mDVelocity.CopyFrom(command, *this); });

  mVelocityDiffCmd.Record(
      [&](Renderer::CommandEncoder& command)
      {
        command.DebugMarkerBegin("Velocity diff", {0.32f, 0.60f, 0.67f, 1.0f});
        mVelocityDiffBound.Record(command);
        mOutputVelocity.Barrier(command,
                                Renderer::ImageLayout::General,
                                Renderer::Access::Write,
                                Renderer::ImageLayout::General,
                                Renderer::Access::Read);
        mDVelocity.CopyFrom(command, mOutputVelocity);
        command.DebugMarkerEnd();
      });
}

Renderer::Texture& Velocity::Output()
{
  return mOutputVelocity;
}

Renderer::Texture& Velocity::D()
{
  return mDVelocity;
}

void Velocity::CopyBack(Renderer::CommandEncoder& command)
{
  CopyFrom(command, mOutputVelocity);
}

void Velocity::Clear(Renderer::CommandEncoder& command)
{
  RenderTexture::Clear(command, std::array<float, 4>{0.0f, 0.0f, 0.0f, 0.0f});
}

void Velocity::SaveCopy()
{
  mSaveCopyCmd.Submit();
}

void Velocity::VelocityDiff()
{
  mVelocityDiffCmd.Submit();
}

}  // namespace Fluid
}  // namespace Vortex
