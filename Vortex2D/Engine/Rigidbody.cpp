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
                     const glm::vec2& centre)
    : mDevice(device)
    , mPhi(device, dimensions.Size.x, dimensions.Size.y, vk::Format::eR32Sfloat)
    , mDrawable(drawable)
    , mCentre(centre)
    , mView(dimensions.InvScale)
    , mVelocity(device)
    , mMVBuffer(device, VMA_MEMORY_USAGE_CPU_TO_GPU)
    , mDiv(device, dimensions.Size, BuildRigidbodyDiv_comp)
    , mConstrain(device, dimensions.Size, ConstrainRigidbodyVelocity_comp)
    , mPressure(device, dimensions.Size, Pressure_comp)
    , mDivCmd(device)
    , mConstrainCmd(device)
    , mPressureCmd(device)
{

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

RigidBody::Velocity RigidBody::GetVelocities() const
{
    Renderer::UniformBuffer<Velocity> localVelocity(mDevice, VMA_MEMORY_USAGE_CPU_ONLY);

    Velocity velocity;
    Renderer::CopyTo(localVelocity, velocity);

    return velocity;
}

void RigidBody::UpdatePosition()
{

}

Renderer::RenderCommand RigidBody::RecordLocalPhi()
{
    return mPhi.Record({mDrawable});
}

Renderer::RenderCommand RigidBody::RecordPhi(Renderer::RenderTexture& phi)
{
    return phi.Record({mDrawable}, UnionBlend);
}

void RigidBody::BindDiv(Renderer::GenericBuffer& div,
                        Renderer::GenericBuffer& diagonal,
                        Renderer::Texture& fluidLevelSet)
{
    mDivBound = mDiv.Bind({div, diagonal, fluidLevelSet, mPhi, mVelocity, mMVBuffer});
    mDivCmd.Record([&](vk::CommandBuffer commandBuffer)
    {
        mDivBound.PushConstant(commandBuffer, 8, mCentre);
        mDivBound.Record(commandBuffer);
    });
}

void RigidBody::BindVelocityConstrain(Renderer::GenericBuffer& velocity)
{

}

void RigidBody::BindPressure(Renderer::GenericBuffer& pressure)
{

}

void RigidBody::Div()
{
    mDivCmd.Submit();
}

void RigidBody::VelocityConstrain()
{

}

}}
