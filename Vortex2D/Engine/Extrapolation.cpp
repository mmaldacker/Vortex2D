//
//  Extrapolation.cpp
//  Vortex2D

#include "Extrapolation.h"

#include "vortex2d_generated_spirv.h"

namespace Vortex2D
{
namespace Fluid
{
Extrapolation::Extrapolation(const Renderer::Device& device,
                             const glm::ivec2& size,
                             Renderer::GenericBuffer& valid,
                             Velocity& velocity,
                             int iterations)
    : mDevice(device)
    , mValid(device, size.x * size.y)
    , mVelocity(velocity)
    , mExtrapolateVelocity(device, size, SPIRV::ExtrapolateVelocity_comp)
    , mExtrapolateVelocityBound(
          mExtrapolateVelocity.Bind({valid, mValid, velocity, velocity.Output()}))
    , mExtrapolateVelocityBackBound(
          mExtrapolateVelocity.Bind({mValid, valid, velocity.Output(), velocity}))
    , mConstrainVelocity(device, size, SPIRV::ConstrainVelocity_comp)
    , mExtrapolateCmd(device, false)
    , mConstrainCmd(device, false)
{
  mExtrapolateCmd.Record([&, iterations](vk::CommandBuffer commandBuffer) {
    commandBuffer.debugMarkerBeginEXT({"Extrapolate", {{0.60f, 0.87f, 0.12f, 1.0f}}}, mDevice.Loader());
    for (int i = 0; i < iterations / 2; i++)
    {
      mExtrapolateVelocityBound.Record(commandBuffer);
      velocity.Output().Barrier(commandBuffer,
                                vk::ImageLayout::eGeneral,
                                vk::AccessFlagBits::eShaderWrite,
                                vk::ImageLayout::eGeneral,
                                vk::AccessFlagBits::eShaderRead);
      mValid.Barrier(
          commandBuffer, vk::AccessFlagBits::eShaderWrite, vk::AccessFlagBits::eShaderRead);
      mExtrapolateVelocityBackBound.Record(commandBuffer);
      velocity.Barrier(commandBuffer,
                       vk::ImageLayout::eGeneral,
                       vk::AccessFlagBits::eShaderWrite,
                       vk::ImageLayout::eGeneral,
                       vk::AccessFlagBits::eShaderRead);
      valid.Barrier(
          commandBuffer, vk::AccessFlagBits::eShaderWrite, vk::AccessFlagBits::eShaderRead);
    }
    commandBuffer.debugMarkerEndEXT(mDevice.Loader());
  });
}

void Extrapolation::Extrapolate()
{
  mExtrapolateCmd.Submit();
}

void Extrapolation::ConstrainBind(Renderer::Texture& solidPhi)
{
  mConstrainVelocityBound = mConstrainVelocity.Bind({solidPhi, mVelocity, mVelocity.Output()});

  mConstrainCmd.Record([&](vk::CommandBuffer commandBuffer) {
    commandBuffer.debugMarkerBeginEXT({"Constrain Velocity", {{0.82f, 0.20f, 0.20f, 1.0f}}}, mDevice.Loader());
    mConstrainVelocityBound.Record(commandBuffer);
    mVelocity.CopyBack(commandBuffer);
    commandBuffer.debugMarkerEndEXT(mDevice.Loader());
  });
}

void Extrapolation::ConstrainVelocity()
{
  mConstrainCmd.Submit();
}

}  // namespace Fluid
}  // namespace Vortex2D
