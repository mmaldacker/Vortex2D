//
//  World.cpp
//  Vortex2D
//

#include "World.h"

#include <glm/gtx/transform.hpp>

namespace Vortex2D { namespace Fluid {

World::World(const Renderer::Device& device, Dimensions dimensions, float dt)
    : mDevice(device)
    , mDimensions(dimensions)
    , mPreconditioner(device, dimensions.Size, dt)
    , mLinearSolver(device, dimensions.Size, mPreconditioner)
    , mData(device, dimensions.Size)
    , mVelocity(device, dimensions.Size)
    , mFluidPhi(device, dimensions.Size)
    , mStaticSolidPhi(device, dimensions.Size)
    , mDynamicSolidPhi(device, dimensions.Size)
    , mValid(device, dimensions.Size.x*dimensions.Size.y)
    , mAdvection(device, dimensions.Size, dt, mVelocity)
    , mProjection(device, dt, dimensions.Size,
                  mData,
                  mVelocity,
                  mDynamicSolidPhi,
                  mFluidPhi,
                  mValid)
    , mExtrapolation(device, dimensions.Size, mValid, mVelocity)
    , mClearValid(device, false)
    , mCopySolidPhi(device, false)
    , mForce(device, dimensions.Size.x*dimensions.Size.y)
{
    mExtrapolation.ConstrainBind(mDynamicSolidPhi);
    mFluidPhi.ExtrapolateBind(mDynamicSolidPhi);

    mClearValid.Record([&](vk::CommandBuffer commandBuffer)
    {
        mValid.Clear(commandBuffer);
    });

    mCopySolidPhi.Record([&](vk::CommandBuffer commandBuffer)
    {
        mDynamicSolidPhi.CopyFrom(commandBuffer, mStaticSolidPhi);
    });

    mPreconditioner.BuildHierarchiesBind(mProjection, mDynamicSolidPhi, mFluidPhi);
    mLinearSolver.Bind(mData.Diagonal, mData.Lower, mData.B, mData.X);

    mFluidPhi.View = dimensions.InvScale;
    mDynamicSolidPhi.View = dimensions.InvScale;
    mStaticSolidPhi.View = dimensions.InvScale;
    mVelocity.Input().View = dimensions.InvScale;

    Renderer::ExecuteCommand(mDevice, [&](vk::CommandBuffer commandBuffer)
    {
        mStaticSolidPhi.Clear(commandBuffer, std::array<float, 4>{{1000.0f, 0.0f, 0.0f, 0.0f}});
    });
}

Renderer::RenderTexture& World::Velocity()
{
    return mVelocity.Input();
}

LevelSet& World::LiquidPhi()
{
    return mFluidPhi;
}

LevelSet& World::StaticSolidPhi()
{
    return mStaticSolidPhi;
}

LevelSet& World::DynamicSolidPhi()
{
    return mDynamicSolidPhi;
}

RigidBody* World::CreateRigidbody(vk::Flags<RigidBody::Type> type, ObjectDrawable& drawable, const glm::vec2& centre)
{
    mRigidbodies.push_back(std::make_unique<RigidBody>(mDevice, mDimensions, drawable, centre, mDynamicSolidPhi, type));

    if (type & RigidBody::Type::eStatic)
    {
        mRigidbodies.back()->BindDiv(mData.B, mData.Diagonal, mFluidPhi);
        mRigidbodies.back()->BindVelocityConstrain(mVelocity);
    }

    if (type & RigidBody::Type::eWeak)
    {
        mRigidbodies.back()->BindPressure(mFluidPhi, mData.X, mForce);
    }

    return mRigidbodies.back().get();
}

SmokeWorld::SmokeWorld(const Renderer::Device& device, Dimensions dimensions, float dt)
    : World(device, dimensions, dt)
{
}

void SmokeWorld::Solve()
{
    mCopySolidPhi.Submit();
    for (auto&& rigidbody: mRigidbodies)
    {
        rigidbody->RenderPhi();
    }

    mDynamicSolidPhi.Reinitialise();
    mPreconditioner.BuildHierarchies();
    mProjection.BuildLinearEquation();

    for (auto&& rigidbody: mRigidbodies)
    {
        if (rigidbody.get()->GetType() & RigidBody::Type::eStatic)
        {
            rigidbody->Div();
        }
    }

    LinearSolver::Parameters params(300, 1e-3f);
    mLinearSolver.Solve(params);
    mProjection.ApplyPressure();

    for (auto&& rigidbody: mRigidbodies)
    {
        if (rigidbody.get()->GetType() & RigidBody::Type::eWeak)
        {
            rigidbody.get()->Pressure();
        }
    }

    mExtrapolation.Extrapolate();
    mExtrapolation.ConstrainVelocity();

    for (auto&& rigidbody: mRigidbodies)
    {
        if (rigidbody.get()->GetType() & RigidBody::Type::eStatic)
        {
            rigidbody.get()->VelocityConstrain();
        }
    }

    mAdvection.AdvectVelocity();
    mAdvection.Advect();

    mClearValid.Submit();
}

void SmokeWorld::FieldBind(Density& density)
{
    mAdvection.AdvectBind(density);
}

WaterWorld::WaterWorld(const Renderer::Device& device, Dimensions dimensions, float dt)
    : World(device, dimensions, dt)
    , mParticles(device, vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eVertexBuffer, VMA_MEMORY_USAGE_GPU_ONLY, 8*dimensions.Size.x*dimensions.Size.y*sizeof(Particle))
    , mParticleCount(device, dimensions.Size, mParticles)
    , mClearVelocity(device)
{
    mParticleCount.LevelSetBind(mFluidPhi);
    mParticleCount.VelocitiesBind(mVelocity, mValid);
    mAdvection.AdvectParticleBind(mParticles, mDynamicSolidPhi, mParticleCount.GetDispatchParams());

    mClearVelocity.Record([&](vk::CommandBuffer commandBuffer)
    {
        mVelocity.Clear(commandBuffer);
    });

    mParticleCount.View = dimensions.InvScale;
}

void WaterWorld::Solve()
{
    /*
     1) From particles, construct fluid level set
     2) Transfer velocities from particles to grid
     3) Add forces to velocity (e.g. gravity)
     4) Construct solid level set and solid velocity fields
     5) Solve pressure, extrapolate and constrain velocities
     6) Update particle velocities with PIC/FLIP
     7) Advect particles
     */

    // 1)
    mParticleCount.Scan();
    mParticleCount.Phi();

    // 2)
    mParticleCount.TransferToGrid();
    mExtrapolation.Extrapolate();

    // 3)
    // transfer to grid adds to the velocity, so we can set the values before

    // 4)
    mCopySolidPhi.Submit();
    for (auto&& rigidbody: mRigidbodies)
    {
        rigidbody->RenderPhi();
    }
    mDynamicSolidPhi.Reinitialise();

    for (auto&& rigidbody: mRigidbodies)
    {
        if (rigidbody.get()->GetType() & RigidBody::Type::eStatic)
        {
            rigidbody->Div();
        }
    }

    mPreconditioner.BuildHierarchies();
    mFluidPhi.Extrapolate();

    // 5)
    mProjection.BuildLinearEquation();
    LinearSolver::Parameters params(1000, 1e-5f);
    mLinearSolver.Solve(params);
    mProjection.ApplyPressure();

    for (auto&& rigidbody: mRigidbodies)
    {
        if (rigidbody.get()->GetType() & RigidBody::Type::eWeak)
        {
            rigidbody.get()->Pressure();
        }
    }

    mExtrapolation.Extrapolate();
    mExtrapolation.ConstrainVelocity();

    for (auto&& rigidbody: mRigidbodies)
    {
        if (rigidbody.get()->GetType() & RigidBody::Type::eStatic)
        {
            rigidbody.get()->VelocityConstrain();
        }
    }

    // 6)
    mParticleCount.TransferFromGrid();

    // 7)
    mAdvection.AdvectParticles();
    mClearVelocity.Submit();
    mClearValid.Submit();
}

Renderer::GenericBuffer& WaterWorld::Particles()
{
    return mParticles;
}

ParticleCount& WaterWorld::Count()
{
    return mParticleCount;
}

}}
