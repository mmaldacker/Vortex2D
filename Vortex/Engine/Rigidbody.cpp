//
//  Rigidbody.cpp
//  Vortex
//

#include "Rigidbody.h"
#include <Vortex/Engine/Boundaries.h>
#include <Vortex/Renderer/CommandBuffer.h>
#include <Vortex/SPIRV/Reflection.h>

#include "vortex_generated_spirv.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/transform.hpp>

namespace Vortex
{
namespace Fluid
{
RigidBody::RigidBody(const Renderer::Device& device,
                     const glm::ivec2& size,
                     Renderer::DrawablePtr drawable,
                     vk::Flags<Type> type)
    : mSize(static_cast<float>(size.x))
    , mDevice(device)
    , mDrawable(drawable)
    , mPhi(device, size.x, size.y, vk::Format::eR32Sfloat)
    , mVelocity(device)
    , mForce(device, size.x * size.y)
    , mReducedForce(device, 1)
    , mLocalForce(device, 1, VMA_MEMORY_USAGE_GPU_TO_CPU)
    , mCenter(device, VMA_MEMORY_USAGE_CPU_TO_GPU)
    , mLocalVelocity(device, VMA_MEMORY_USAGE_CPU_ONLY)
    , mDiv(device, size, SPIRV::BuildRigidbodyDiv_comp)
    , mConstrain(device, size, SPIRV::ConstrainRigidbodyVelocity_comp)
    , mForceWork(device, size, SPIRV::RigidbodyForce_comp)
    , mPressureWork(device, size, SPIRV::RigidbodyPressure_comp)
    , mDivCmd(device, false)
    , mConstrainCmd(device, false)
    , mForceCmd(device, true)
    , mPressureCmd(device, false)
    , mVelocityCmd(device, false)
    , mSum(device, size)
    , mType(type)
    , mMass(0.0f)
    , mInertia(0.0f)
{
  mLocalPhiRender = mPhi.Record({BoundariesClear, drawable}, UnionBlend);

  mVelocityCmd.Record([&](vk::CommandBuffer commandBuffer)
                      { mVelocity.CopyFrom(commandBuffer, mLocalVelocity); });

  SetVelocities(glm::vec2(0.0f), 0.0f);
}

RigidBody::~RigidBody() {}

void RigidBody::ApplyForces() {}

void RigidBody::ApplyVelocities() {}

void RigidBody::SetMassData(float mass, float inertia)
{
  mMass = mass;
  mInertia = inertia;
}

void RigidBody::SetVelocities(const glm::vec2& velocity, float angularVelocity)
{
  Velocity v{velocity / glm::vec2(mSize), angularVelocity};

  Renderer::CopyFrom(mLocalVelocity, v);
  mVelocityCmd.Submit();
}

RigidBody::Velocity RigidBody::GetForces()
{
  mForceCmd.Wait();

  Velocity force;
  Renderer::CopyTo(mLocalForce, force);

  force.velocity *= glm::vec2(mSize);
  force.angular_velocity *= mSize * mSize;

  return force;
}

void RigidBody::UpdatePosition()
{
  Renderer::CopyFrom(mCenter, Position);
}

void RigidBody::RenderPhi()
{
  Transformable::Update();
  mLocalPhiRender.Submit(GetTransform());
  mPhiRender.Submit(GetTransform());
}

void RigidBody::BindPhi(Renderer::RenderTexture& phi)
{
  mPhiRender = phi.Record({mDrawable}, UnionBlend);
}

void RigidBody::BindDiv(Renderer::GenericBuffer& div, Renderer::GenericBuffer& diagonal)
{
  mDivBound = mDiv.Bind({div, diagonal, mPhi, mVelocity, mCenter});
  mDivCmd.Record(
      [&](vk::CommandBuffer commandBuffer)
      {
        commandBuffer.debugMarkerBeginEXT(
            {"Rigidbody build equation", {{0.90f, 0.27f, 0.28f, 1.0f}}}, mDevice.Loader());
        mDivBound.Record(commandBuffer);
        div.Barrier(
            commandBuffer, vk::AccessFlagBits::eShaderWrite, vk::AccessFlagBits::eShaderRead);
        commandBuffer.debugMarkerEndEXT(mDevice.Loader());
      });
}

void RigidBody::BindVelocityConstrain(Fluid::Velocity& velocity)
{
  mConstrainBound = mConstrain.Bind({velocity, velocity.Output(), mPhi, mVelocity, mCenter});
  mConstrainCmd.Record(
      [&](vk::CommandBuffer commandBuffer)
      {
        commandBuffer.debugMarkerBeginEXT({"Rigidbody constrain", {{0.29f, 0.36f, 0.21f, 1.0f}}},
                                          mDevice.Loader());
        mConstrainBound.Record(commandBuffer);
        velocity.CopyBack(commandBuffer);
        commandBuffer.debugMarkerEndEXT(mDevice.Loader());
      });
}

void RigidBody::BindForce(Renderer::GenericBuffer& diagonal, Renderer::GenericBuffer& pressure)
{
  mForceBound = mForceWork.Bind({diagonal, mPhi, pressure, mForce, mCenter});
  mLocalSumBound = mSum.Bind(mForce, mLocalForce);
  mForceCmd.Record(
      [&](vk::CommandBuffer commandBuffer)
      {
        commandBuffer.debugMarkerBeginEXT({"Rigidbody force", {{0.70f, 0.59f, 0.63f, 1.0f}}},
                                          mDevice.Loader());
        mForce.Clear(commandBuffer);
        mForceBound.Record(commandBuffer);
        mForce.Barrier(
            commandBuffer, vk::AccessFlagBits::eShaderWrite, vk::AccessFlagBits::eShaderRead);
        mLocalSumBound.Record(commandBuffer);
        commandBuffer.debugMarkerEndEXT(mDevice.Loader());
      });
}

void RigidBody::BindPressure(float delta,
                             Renderer::GenericBuffer& d,
                             Renderer::GenericBuffer& s,
                             Renderer::GenericBuffer& z)
{
  mPressureForceBound = mForceWork.Bind({d, mPhi, s, mForce, mCenter});
  mPressureBound = mPressureWork.Bind({d, mPhi, mReducedForce, z, mCenter});
  mSumBound = mSum.Bind(mForce, mReducedForce);
  mPressureCmd.Record(
      [&](vk::CommandBuffer commandBuffer)
      {
        commandBuffer.debugMarkerBeginEXT({"Rigidbody pressure", {{0.70f, 0.59f, 0.63f, 1.0f}}},
                                          mDevice.Loader());
        mForce.Clear(commandBuffer);
        mPressureForceBound.Record(commandBuffer);
        mForce.Barrier(
            commandBuffer, vk::AccessFlagBits::eShaderWrite, vk::AccessFlagBits::eShaderRead);
        mSumBound.Record(commandBuffer);
        mPressureBound.PushConstant(commandBuffer, delta, mMass, mInertia);
        mPressureBound.Record(commandBuffer);
        z.Barrier(commandBuffer, vk::AccessFlagBits::eShaderWrite, vk::AccessFlagBits::eShaderRead);
        commandBuffer.debugMarkerEndEXT(mDevice.Loader());
      });
}

void RigidBody::Div()
{
  if (mType & RigidBody::Type::eStatic)
  {
    mDivCmd.Submit();
  }
}

void RigidBody::Force()
{
  if (mType & RigidBody::Type::eWeak)
  {
    mForceCmd.Submit();
  }
}

void RigidBody::Pressure()
{
  if (mType == RigidBody::Type::eStrong)
  {
    mPressureCmd.Submit();
  }
}

void RigidBody::VelocityConstrain()
{
  if (mType & RigidBody::Type::eStatic)
  {
    mConstrainCmd.Submit();
  }
}

vk::Flags<RigidBody::Type> RigidBody::GetType()
{
  return mType;
}

void RigidBody::SetType(RigidBody::Type type)
{
  mType = type;
}

Renderer::RenderTexture& RigidBody::Phi()
{
  return mPhi;
}

}  // namespace Fluid
}  // namespace Vortex
