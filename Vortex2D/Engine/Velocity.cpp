//
//  Velocity.cpp
//  Vortex2D
//

#include "Velocity.h"

#include "vortex2d_generated_spirv.h"

namespace Vortex2D
{
namespace Fluid
{
Velocity::Velocity(const Renderer::Device& device, const glm::ivec2& size)
    : Renderer::RenderTexture(device, size.x, size.y, vk::Format::eR32G32Sfloat)
    , mOutputVelocity(device, size.x, size.y, vk::Format::eR32G32Sfloat)
    , mDVelocity(device, size.x, size.y, vk::Format::eR32G32Sfloat)
    , mVelocityDiff(device, size, SPIRV::VelocityDifference_comp)
    , mVelocityDiffBound(mVelocityDiff.Bind({mDVelocity, *this, mOutputVelocity}))
    , mSaveCopyCmd(device, false)
    , mVelocityDiffCmd(device, false)
{
  mSaveCopyCmd.Record(
      [&](vk::CommandBuffer commandBuffer) { mDVelocity.CopyFrom(commandBuffer, *this); });

  mVelocityDiffCmd.Record([&](vk::CommandBuffer commandBuffer) {
    commandBuffer.debugMarkerBeginEXT({"Velocity diff", {{0.32f, 0.60f, 0.67f, 1.0f}}});
    mVelocityDiffBound.Record(commandBuffer);
    mOutputVelocity.Barrier(commandBuffer,
                            vk::ImageLayout::eGeneral,
                            vk::AccessFlagBits::eShaderWrite,
                            vk::ImageLayout::eGeneral,
                            vk::AccessFlagBits::eShaderRead);
    mDVelocity.CopyFrom(commandBuffer, mOutputVelocity);
    commandBuffer.debugMarkerEndEXT();
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

void Velocity::CopyBack(vk::CommandBuffer commandBuffer)
{
  CopyFrom(commandBuffer, mOutputVelocity);
}

void Velocity::Clear(vk::CommandBuffer commandBuffer)
{
  RenderTexture::Clear(commandBuffer, std::array<float, 4>{0.0f, 0.0f, 0.0f, 0.0f});
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
}  // namespace Vortex2D
