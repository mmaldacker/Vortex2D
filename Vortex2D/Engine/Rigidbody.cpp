//
//  Rigidbody.cpp
//  Vortex2D
//

#include "Rigidbody.h"
#include <Vortex2D/SPIRV/Reflection.h>
#include <Vortex2D/Renderer/CommandBuffer.h>
#include <Vortex2D/Engine/Boundaries.h>

#include "vortex2d_generated_spirv.h"

namespace Vortex2D { namespace Fluid {

RigidBody::RigidBody(const Renderer::Device& device,
                     const Dimensions& dimensions,
                     Renderer::Drawable& drawable,
                     const glm::vec2& centre,
                     Renderer::RenderTexture& phi,
                     vk::Flags<Type> type)
    : mDevice(device)
    , mPhi(device, dimensions.Size.x, dimensions.Size.y, vk::Format::eR32Sfloat)
    , mDrawable(drawable)
    , mCentre(centre)
    , mView(dimensions.InvScale)
    , mVelocity(device)
    , mForce(device, 1, VMA_MEMORY_USAGE_GPU_TO_CPU)
    , mMVBuffer(device, VMA_MEMORY_USAGE_CPU_TO_GPU)
    , mClear({1000.0f, 0.0f, 0.0f, 0.0f})
    , mDiv(device, dimensions.Size, SPIRV::BuildRigidbodyDiv_comp)
    , mConstrain(device, dimensions.Size, SPIRV::ConstrainRigidbodyVelocity_comp)
    , mPressure(device, dimensions.Size, SPIRV::RigidbodyPressure_comp)
    , mDivCmd(device, false)
    , mConstrainCmd(device, false)
    , mPressureCmd(device, false)
    , mSum(device, dimensions.Size)
    , mType(type)
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
    Velocity v{velocity, angularVelocity};

    Renderer::UniformBuffer<Velocity> localVelocity(mDevice, VMA_MEMORY_USAGE_CPU_ONLY);
    Renderer::CopyFrom(localVelocity, v);

    Renderer::ExecuteCommand(mDevice, [&](vk::CommandBuffer commandBuffer)
    {
        mVelocity.CopyFrom(commandBuffer, localVelocity);
    });
}

RigidBody::Velocity RigidBody::GetForces()
{
    Velocity force;
    Renderer::CopyTo(mForce, force);

    return force;
}

void RigidBody::UpdatePosition()
{
    Renderer::CopyFrom(mMVBuffer, mView * GetTransform());
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
                        Renderer::GenericBuffer& diagonal,
                        Renderer::Texture& fluidLevelSet)
{
    mDivBound = mDiv.Bind({div, diagonal, fluidLevelSet, mPhi, mVelocity, mMVBuffer});
    mDivCmd.Record([&](vk::CommandBuffer commandBuffer)
    {
        commandBuffer.debugMarkerBeginEXT({"Rigidbody build equation", {{ 0.90f, 0.27f, 0.28f, 1.0f}}});
        mDivBound.PushConstant(commandBuffer, 8, mCentre);
        mDivBound.Record(commandBuffer);
        commandBuffer.debugMarkerEndEXT();
    });
}

void RigidBody::BindVelocityConstrain(Fluid::Velocity& velocity)
{
    mConstrainBound = mConstrain.Bind({velocity.Input(), velocity.Output(), mPhi, mVelocity, mMVBuffer});
    mConstrainCmd.Record([&](vk::CommandBuffer commandBuffer)
    {
        commandBuffer.debugMarkerBeginEXT({"Rigidbody constrain", {{ 0.29f, 0.36f, 0.21f, 1.0f}}});
        mConstrainBound.PushConstant(commandBuffer, 8, mCentre);
        mConstrainBound.Record(commandBuffer);
        velocity.CopyBack(commandBuffer);
        commandBuffer.debugMarkerEndEXT();
    });
}

void RigidBody::BindPressure(Renderer::Texture& fluidLevelSet,
                             Renderer::GenericBuffer& pressure,
                             Renderer::GenericBuffer& force)
{
    mPressureBound = mPressure.Bind({fluidLevelSet, mPhi, pressure, force, mMVBuffer});
    mSumBound = mSum.Bind(force, mForce);
    mPressureCmd.Record([&](vk::CommandBuffer commandBuffer)
    {
        commandBuffer.debugMarkerBeginEXT({"Rigidbody pressure", {{ 0.70f, 0.59f, 0.63f, 1.0f}}});
        force.Clear(commandBuffer);
        mPressureBound.PushConstant(commandBuffer, 8, mCentre);
        mPressureBound.Record(commandBuffer);
        force.Barrier(commandBuffer, vk::AccessFlagBits::eShaderWrite, vk::AccessFlagBits::eShaderRead);
        mSumBound.Record(commandBuffer);
        commandBuffer.debugMarkerEndEXT();
    });
}

void RigidBody::Div()
{
    mDivCmd.Submit();
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
