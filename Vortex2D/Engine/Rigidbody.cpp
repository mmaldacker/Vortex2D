//
//  Rigidbody.cpp
//  Vortex2D
//

#include "Rigidbody.h"
#include <Vortex2D/SPIRV/Reflection.h>
#include <Vortex2D/Renderer/CommandBuffer.h>
#include <Vortex2D/Engine/Boundaries.h>

#include "vortex2d_generated_spirv.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/transform.hpp>

namespace Vortex2D { namespace Fluid {

RigidBody::RigidBody(const Renderer::Device& device,
                     const Dimensions& dimensions,
                     float delta,
                     Renderer::Drawable& drawable,
                     const glm::vec2& centre,
                     Renderer::RenderTexture& phi,
                     vk::Flags<Type> type,
                     float mass,
                     float inertia)
    : mScale(dimensions.Scale)
    , mDelta(delta)
    , mDevice(device)
    , mPhi(device, dimensions.Size.x, dimensions.Size.y, vk::Format::eR32Sfloat)
    , mDrawable(drawable)
    , mCentre(centre)
    , mView(dimensions.InvScale)
    , mVelocity(device)
    , mForce(device, dimensions.Size.x * dimensions.Size.y)
    , mReducedForce(device, 1)
    , mLocalForce(device, 1, VMA_MEMORY_USAGE_GPU_TO_CPU)
    , mMVBuffer(device, VMA_MEMORY_USAGE_CPU_TO_GPU)
    , mClear({1000.0f, 0.0f, 0.0f, 0.0f})
    , mDiv(device, dimensions.Size, SPIRV::BuildRigidbodyDiv_comp)
    , mConstrain(device, dimensions.Size, SPIRV::ConstrainRigidbodyVelocity_comp)
    , mForceWork(device, dimensions.Size, SPIRV::RigidbodyForce_comp)
    , mPressureWork(device, dimensions.Size, SPIRV::RigidbodyPressure_comp)
    , mDivCmd(device, false)
    , mConstrainCmd(device, false)
    , mForceCmd(device, false)
    , mPressureCmd(device, false)
    , mSum(device, dimensions.Size)
    , mType(type)
    , mMass(mass)
    , mInertia(inertia)
{
    mPhi.View = mView;
    mLocalPhiRender = mPhi.Record({mClear, mDrawable}, UnionBlend);
    SetVelocities(glm::vec2(0.0f), 0.0f);

    if (type & Type::eStatic)
    {
        mPhiRender = phi.Record({mDrawable}, UnionBlend);
    }
}

void RigidBody::SetVelocities(const glm::vec2& velocity, float angularVelocity)
{
    Velocity v{velocity / glm::vec2(mScale), angularVelocity};

    Renderer::UniformBuffer<Velocity> localVelocity(mDevice, VMA_MEMORY_USAGE_CPU_ONLY);
    Renderer::CopyFrom(localVelocity, v);

    // TODO use command buffer and call wait at the appropriate moment
    Renderer::ExecuteCommand(mDevice, [&](vk::CommandBuffer commandBuffer)
    {
        mVelocity.CopyFrom(commandBuffer, localVelocity);
    });
}

RigidBody::Velocity RigidBody::GetForces()
{
    // TODO looks like we need to wait on the command buffer here
    Velocity force;
    Renderer::CopyTo(mLocalForce, force);

    force.angular_velocity *= mScale * mScale;
    force.velocity *= glm::vec2(mScale * mScale);

    return force;
}

void RigidBody::UpdatePosition()
{
    Renderer::CopyFrom(mMVBuffer, mView * glm::translate(glm::vec3{(glm::vec2)Position, 0.0f}));
}

void RigidBody::RenderPhi()
{
    mLocalPhiRender.Submit(GetTransform());
    if (mType & Type::eStatic)
    {
        mPhiRender.Submit(GetTransform());
    }
}

void RigidBody::BindDiv(Renderer::GenericBuffer& div,
                        Renderer::GenericBuffer& diagonal)
{
    mDivBound = mDiv.Bind({div, diagonal , mPhi, mVelocity, mMVBuffer});
    mDivCmd.Record([&](vk::CommandBuffer commandBuffer)
    {
        commandBuffer.debugMarkerBeginEXT({"Rigidbody build equation", {{0.90f, 0.27f, 0.28f, 1.0f}}});
        mDivBound.PushConstant(commandBuffer, mCentre);
        mDivBound.Record(commandBuffer);
        div.Barrier(commandBuffer, vk::AccessFlagBits::eShaderWrite, vk::AccessFlagBits::eShaderRead);
        commandBuffer.debugMarkerEndEXT();
    });
}

void RigidBody::BindVelocityConstrain(Fluid::Velocity& velocity)
{
    mConstrainBound = mConstrain.Bind({velocity, velocity.Output(), mPhi, mVelocity, mMVBuffer});
    mConstrainCmd.Record([&](vk::CommandBuffer commandBuffer)
    {
        commandBuffer.debugMarkerBeginEXT({"Rigidbody constrain", {{0.29f, 0.36f, 0.21f, 1.0f}}});
        mConstrainBound.PushConstant(commandBuffer, mCentre);
        mConstrainBound.Record(commandBuffer);
        velocity.CopyBack(commandBuffer);
        commandBuffer.debugMarkerEndEXT();
    });
}

void RigidBody::BindForce(Renderer::GenericBuffer& diagonal,
                          Renderer::GenericBuffer& pressure)
{
    mForceBound = mForceWork.Bind({diagonal, mPhi, pressure, mForce, mMVBuffer});
    mLocalSumBound = mSum.Bind(mForce, mLocalForce);
    mForceCmd.Record([&](vk::CommandBuffer commandBuffer)
    {
        commandBuffer.debugMarkerBeginEXT({"Rigidbody force", {{0.70f, 0.59f, 0.63f, 1.0f}}});
        mForce.Clear(commandBuffer);
        mForceBound.PushConstant(commandBuffer, mCentre);
        mForceBound.Record(commandBuffer);
        mForce.Barrier(commandBuffer, vk::AccessFlagBits::eShaderWrite, vk::AccessFlagBits::eShaderRead);
        mLocalSumBound.Record(commandBuffer);
        commandBuffer.debugMarkerEndEXT();
    });
}

void RigidBody::BindPressure(Renderer::GenericBuffer& d,
                             Renderer::GenericBuffer& s,
                             Renderer::GenericBuffer& z)
{
    mPressureForceBound = mForceWork.Bind({d, mPhi, s, mForce, mMVBuffer});
    mPressureBound = mPressureWork.Bind({d, mPhi, mReducedForce, z, mMVBuffer});
    mSumBound = mSum.Bind(mForce, mReducedForce);
    mPressureCmd.Record([&](vk::CommandBuffer commandBuffer)
    {
        commandBuffer.debugMarkerBeginEXT({"Rigidbody pressure", {{0.70f, 0.59f, 0.63f, 1.0f}}});
        mForce.Clear(commandBuffer);
        mPressureForceBound.PushConstant(commandBuffer, mCentre);
        mPressureForceBound.Record(commandBuffer);
        mForce.Barrier(commandBuffer, vk::AccessFlagBits::eShaderWrite, vk::AccessFlagBits::eShaderRead);
        mSumBound.Record(commandBuffer);
        mPressureBound.PushConstant(commandBuffer, mCentre, mDelta, mMass, mInertia);
        mPressureBound.Record(commandBuffer);
        z.Barrier(commandBuffer, vk::AccessFlagBits::eShaderWrite, vk::AccessFlagBits::eShaderRead);
        commandBuffer.debugMarkerEndEXT();
    });
}

void RigidBody::Div()
{
    mDivCmd.Submit();
}

void RigidBody::Force()
{
    mForceCmd.Submit();
}

void RigidBody::Pressure()
{
    mPressureCmd.Submit();
}

void RigidBody::VelocityConstrain()
{
    mConstrainCmd.Submit();
}

vk::Flags<RigidBody::Type> RigidBody::GetType()
{
    return mType;
}

Renderer::RenderTexture& RigidBody::Phi()
{
    return mPhi;
}

}}
