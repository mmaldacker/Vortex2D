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
RigidBody::RigidBody(Renderer::Device& device,
                     const glm::ivec2& size,
                     Renderer::DrawablePtr drawable,
                     Type type)
    : mSize(static_cast<float>(size.x))
    , mDevice(device)
    , mDrawable(drawable)
    , mPhi(device, size.x, size.y, Renderer::Format::R32Sfloat)
    , mVelocity(device)
    , mForce(device, size.x * size.y)
    , mReducedForce(device, 1)
    , mLocalForce(device, 1, Renderer::MemoryUsage::GpuToCpu)
    , mCenter(device, Renderer::MemoryUsage::CpuToGpu)
    , mLocalVelocity(device, Renderer::MemoryUsage::Cpu)
    , mDiv(device, Renderer::ComputeSize{size}, SPIRV::BuildRigidbodyDiv_comp)
    , mConstrain(device, Renderer::ComputeSize{size}, SPIRV::ConstrainRigidbodyVelocity_comp)
    , mForceWork(device, Renderer::ComputeSize{size}, SPIRV::RigidbodyForce_comp)
    , mPressureWork(device, Renderer::ComputeSize{size}, SPIRV::RigidbodyPressure_comp)
    , mDivCmd(device, false)
    , mConstrainCmd(device, false)
    , mForceCmd(device, true)
    , mPressureCmd(device, false)
    , mVelocityCmd(device, false)
    , mSum(device, size.x * size.y)
    , mType(type)
    , mMass(0.0f)
    , mInertia(0.0f)
{
  mLocalPhiRender = mPhi.Record({BoundariesClear, drawable}, UnionBlend);

  mVelocityCmd.Record([&](Renderer::CommandEncoder& command)
                      { mVelocity.CopyFrom(command, mLocalVelocity); });

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
      [&](Renderer::CommandEncoder& command)
      {
        command.DebugMarkerBegin("Rigidbody build equation", {0.90f, 0.27f, 0.28f, 1.0f});
        mDivBound.Record(command);
        div.Barrier(command, Renderer::Access::Write, Renderer::Access::Read);
        command.DebugMarkerEnd();
      });
}

void RigidBody::BindVelocityConstrain(Fluid::Velocity& velocity)
{
  mConstrainBound = mConstrain.Bind({velocity, velocity.Output(), mPhi, mVelocity, mCenter});
  mConstrainCmd.Record(
      [&](Renderer::CommandEncoder& command)
      {
        command.DebugMarkerBegin("Rigidbody constrain", {0.29f, 0.36f, 0.21f, 1.0f});
        mConstrainBound.Record(command);
        velocity.CopyBack(command);
        command.DebugMarkerEnd();
      });
}

void RigidBody::BindForce(Renderer::GenericBuffer& diagonal, Renderer::GenericBuffer& pressure)
{
  mForceBound = mForceWork.Bind({diagonal, mPhi, pressure, mForce, mCenter});
  mLocalSumBound = mSum.Bind(mForce, mLocalForce);
  mForceCmd.Record(
      [&](Renderer::CommandEncoder& command)
      {
        command.DebugMarkerBegin("Rigidbody force", {0.70f, 0.59f, 0.63f, 1.0f});
        mForce.Clear(command);
        mForceBound.Record(command);
        mForce.Barrier(command, Renderer::Access::Write, Renderer::Access::Read);
        mLocalSumBound.Record(command);
        command.DebugMarkerEnd();
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
      [&](Renderer::CommandEncoder& command)
      {
        command.DebugMarkerBegin("Rigidbody pressure", {0.70f, 0.59f, 0.63f, 1.0f});
        mForce.Clear(command);
        mPressureForceBound.Record(command);
        mForce.Barrier(command, Renderer::Access::Write, Renderer::Access::Read);
        mSumBound.Record(command);
        mPressureBound.PushConstant(command, delta, mMass, mInertia);
        mPressureBound.Record(command);
        z.Barrier(command, Renderer::Access::Write, Renderer::Access::Read);
        command.DebugMarkerEnd();
      });
}

void RigidBody::Div()
{
  if (mType == RigidBody::Type::eStatic || mType == RigidBody::Type::eStrong)
  {
    mDivCmd.Submit();
  }
}

void RigidBody::Force()
{
  if (mType == RigidBody::Type::eWeak || mType == Vortex::Fluid::RigidBody::Type::eStrong)
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
  if (mType == RigidBody::Type::eStatic || mType == RigidBody::Type::eStrong)
  {
    mConstrainCmd.Submit();
  }
}

RigidBody::Type RigidBody::GetType()
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
