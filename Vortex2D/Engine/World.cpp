//
//  World.cpp
//  Vortex2D
//

#include "World.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/transform.hpp>

namespace Vortex2D { namespace Fluid {

std::vector<RigidBody*> GetRigidbodyPointers(const std::vector<std::unique_ptr<RigidBody>>& rigidbodies)
{
    std::vector<RigidBody*> rigidBodiesPointers;
    for (auto& rigibody: rigidbodies)
    {
        rigidBodiesPointers.push_back(rigibody.get());
    }

    return rigidBodiesPointers;
}

void ForAll(std::vector<std::unique_ptr<RigidBody>>& rigidbodies, void(RigidBody::*f)())
{
    for (auto& rigidbody: rigidbodies)
    {
        (*(rigidbody).*f)();
    }
}

World::World(const Renderer::Device& device, const glm::ivec2& size, float dt, int numSubSteps)
    : mDevice(device)
    , mSize(size)
    , mDelta(dt / numSubSteps)
    , mNumSubSteps(numSubSteps)
    , mPreconditioner(device, size, mDelta)
    , mLinearSolver(device, size, mPreconditioner)
    , mData(device, size)
    , mVelocity(device, size)
    , mLiquidPhi(device, size)
    , mStaticSolidPhi(device, size)
    , mDynamicSolidPhi(device, size)
    , mValid(device, size.x*size.y)
    , mAdvection(device, size, mDelta, mVelocity)
    , mProjection(device, mDelta, size,
                  mData,
                  mVelocity,
                  mDynamicSolidPhi,
                  mLiquidPhi,
                  mValid)
    , mExtrapolation(device, size, mValid, mVelocity)
    , mCopySolidPhi(device, false)
    , mCfl(device, size, mVelocity)
{
    mExtrapolation.ConstrainBind(mDynamicSolidPhi);
    mLiquidPhi.ExtrapolateBind(mDynamicSolidPhi);

    mCopySolidPhi.Record([&](vk::CommandBuffer commandBuffer)
    {
        mDynamicSolidPhi.CopyFrom(commandBuffer, mStaticSolidPhi);
    });

    mPreconditioner.BuildHierarchiesBind(mProjection, mDynamicSolidPhi, mLiquidPhi);
    mLinearSolver.Bind(mData.Diagonal, mData.Lower, mData.B, mData.X);

    Renderer::ExecuteCommand(mDevice, [&](vk::CommandBuffer commandBuffer)
    {
        mStaticSolidPhi.Clear(commandBuffer, std::array<float, 4>{{10000.0f, 0.0f, 0.0f, 0.0f}});
    });
}

void World::Step(LinearSolver::Parameters& params)
{
    for (int i = 0; i < mNumSubSteps; i++)
    {
        Substep(params);
    }
}

Renderer::RenderCommand World::RecordVelocity(Renderer::RenderTarget::DrawableList drawables)
{
    Renderer::ColorBlendState blendState;
    blendState.ColorBlend
            .setBlendEnable(true)
            .setColorBlendOp(vk::BlendOp::eAdd)
            .setSrcColorBlendFactor(vk::BlendFactor::eConstantColor)
            .setDstColorBlendFactor(vk::BlendFactor::eOne);

    float scale = 1.0f / mSize.x;
    blendState.BlendConstants = {scale, scale, scale, scale};

    return mVelocity.Record(drawables, blendState);
}

void World::SubmitVelocity(Renderer::RenderCommand& renderCommand)
{
    mVelocities.push_back({renderCommand});
}

Renderer::RenderCommand World::RecordLiquidPhi(Renderer::RenderTarget::DrawableList drawables)
{
    return mLiquidPhi.Record(drawables);
}

Renderer::RenderCommand World::RecordStaticSolidPhi(Renderer::RenderTarget::DrawableList drawables)
{
    return mStaticSolidPhi.Record(drawables, UnionBlend);
}

DistanceField  World::LiquidDistanceField()
{
    return  {mDevice, mLiquidPhi};
}

DistanceField  World::SolidDistanceField()
{
    return {mDevice, mDynamicSolidPhi};
}

RigidBody* World::CreateRigidbody(vk::Flags<RigidBody::Type> type, float mass, float inertia, Renderer::Drawable& drawable, const glm::vec2& centre)
{
    mRigidbodies.push_back(std::make_unique<RigidBody>(mDevice, mSize, mDelta, drawable, centre, mDynamicSolidPhi, type, mass, inertia));

    if (type & RigidBody::Type::eStatic)
    {
        mRigidbodies.back()->BindDiv(mData.B, mData.Diagonal);
        mRigidbodies.back()->BindVelocityConstrain(mVelocity);
        mLinearSolver.BindRigidbody(mData.Diagonal, *mRigidbodies.back());
    }

    if (type & RigidBody::Type::eWeak)
    {
        mRigidbodies.back()->BindForce(mData.Diagonal, mData.X);
    }

    return mRigidbodies.back().get();
}

float World::GetCFL()
{
    mCfl.Compute();
    return mCfl.Get();
}

Renderer::Texture& World::GetVelocity()
{
    return mVelocity;
}

SmokeWorld::SmokeWorld(const Renderer::Device& device, const glm::ivec2& size, float dt)
    : World(device, size, dt)
{
}

void SmokeWorld::Substep(LinearSolver::Parameters& params)
{
    for (auto& velocity: mVelocities)
    {
        velocity.get().Submit();
    }
    mVelocities.clear();

    mCopySolidPhi.Submit();

    ForAll(mRigidbodies, &RigidBody::RenderPhi);

    mDynamicSolidPhi.Reinitialise();
    mPreconditioner.BuildHierarchies();
    mProjection.BuildLinearEquation();

    ForAll(mRigidbodies, &RigidBody::Div);

    mLinearSolver.Solve(params, GetRigidbodyPointers(mRigidbodies));
    mProjection.ApplyPressure();

    ForAll(mRigidbodies, &RigidBody::Force);

    mExtrapolation.Extrapolate();
    mExtrapolation.ConstrainVelocity();

    ForAll(mRigidbodies, &RigidBody::VelocityConstrain);

    mAdvection.AdvectVelocity();
    mAdvection.Advect();
}

void SmokeWorld::FieldBind(Density& density)
{
    mAdvection.AdvectBind(density);
}

WaterWorld::WaterWorld(const Renderer::Device& device, const glm::ivec2& size, float dt, int numSubSteps)
    : World(device, size, dt, numSubSteps)
    , mParticles(device, vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eVertexBuffer, VMA_MEMORY_USAGE_GPU_ONLY, 8*size.x*size.y*sizeof(Particle))
    , mParticleCount(device, size, mParticles, {0}, 0.02f)
{
    mParticleCount.LevelSetBind(mLiquidPhi);
    mParticleCount.VelocitiesBind(mVelocity, mValid);
    mAdvection.AdvectParticleBind(mParticles, mDynamicSolidPhi, mParticleCount.GetDispatchParams());
}

void WaterWorld::Substep(LinearSolver::Parameters& params)
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
    mVelocity.SaveCopy();

    // 3)
    for (auto& velocity: mVelocities)
    {
        velocity.get().Submit();
    }
    mVelocities.clear();

    // 4)
    mCopySolidPhi.Submit();
    ForAll(mRigidbodies, &RigidBody::RenderPhi);
    mDynamicSolidPhi.Reinitialise();

    ForAll(mRigidbodies, &RigidBody::Div);

    mPreconditioner.BuildHierarchies();
    mLiquidPhi.Extrapolate();

    // 5)
    mProjection.BuildLinearEquation();
    mLinearSolver.Solve(params, GetRigidbodyPointers(mRigidbodies));
    mProjection.ApplyPressure();

    ForAll(mRigidbodies, &RigidBody::Force);

    mExtrapolation.Extrapolate();
    mExtrapolation.ConstrainVelocity();

    ForAll(mRigidbodies, &RigidBody::VelocityConstrain);

    // 6)
    mVelocity.VelocityDiff();
    mParticleCount.TransferFromGrid();

    // 7)
    mAdvection.AdvectParticles();
}

Renderer::RenderCommand WaterWorld::RecordParticleCount(Renderer::RenderTarget::DrawableList drawables)
{
    return mParticleCount.Record(drawables);
}

}}
