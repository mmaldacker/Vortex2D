//
//  World.cpp
//  Vortex2D
//

#include "World.h"

#include <glm/gtx/transform.hpp>

namespace Vortex2D { namespace Fluid {

World::World(const Renderer::Device& device, Dimensions dimensions, float dt)
    : mDimensions(dimensions)
    , mParticles(device, vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eVertexBuffer, VMA_MEMORY_USAGE_GPU_ONLY, 8*dimensions.Size.x*dimensions.Size.y*sizeof(Particle))
    , mParticleCount(device, dimensions.Size, mParticles)
    , mPreconditioner(device, dimensions.Size, dt)
    , mLinearSolver(device, dimensions.Size, mPreconditioner)
    , mData(device, dimensions.Size)
    , mVelocity(device, dimensions.Size.x, dimensions.Size.y, vk::Format::eR32G32Sfloat)
    , mFluidLevelSet(device, dimensions.Size)
    , mObstacleLevelSet(device, dimensions.Size)
    , mValid(device, dimensions.Size.x*dimensions.Size.y)
    , mAdvection(device, dimensions.Size, dt, mVelocity)
    , mProjection(device, dt, dimensions.Size,
                  mData,
                  mVelocity,
                  mObstacleLevelSet,
                  mFluidLevelSet,
                  mValid)
    , mExtrapolation(device, dimensions.Size, mValid, mVelocity)
    , mClearVelocity(device, false)
    , mClearValid(device, false)
{
    mExtrapolation.ConstrainInit(mObstacleLevelSet);
    mParticleCount.InitLevelSet(mFluidLevelSet);
    mParticleCount.InitVelocities(mVelocity, mValid);
    mFluidLevelSet.ExtrapolateInit(mObstacleLevelSet);
    mAdvection.AdvectParticleInit(mParticles, mObstacleLevelSet, mParticleCount.GetDispatchParams());

    mClearVelocity.Record([&](vk::CommandBuffer commandBuffer)
    {
        mVelocity.Clear(commandBuffer, std::array<float, 4>{0.0f, 0.0f, 0.0f, 0.0f});
    });

    mClearValid.Record([&](vk::CommandBuffer commandBuffer)
    {
        mValid.Clear(commandBuffer);
    });

    mPreconditioner.BuildHierarchiesInit(mProjection, mObstacleLevelSet, mFluidLevelSet);
    mLinearSolver.Init(mData.Diagonal, mData.Lower, mData.B, mData.X);
}

void World::InitField(Density& density)
{
    mAdvection.AdvectInit(density);
}

void World::SolveStatic()
{
    LinearSolver::Parameters params(300, 1e-3f);
    mPreconditioner.BuildHierarchies();
    mProjection.BuildLinearEquation();
    mLinearSolver.Solve(params);
    mProjection.ApplyPressure();

    mExtrapolation.Extrapolate();
    mExtrapolation.ConstrainVelocity();

    mAdvection.AdvectVelocity();
    mAdvection.Advect();

    mClearValid.Submit();
}

void World::SolveDynamic()
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
    mPreconditioner.BuildHierarchies();
    mFluidLevelSet.Extrapolate();

    // 5)
    LinearSolver::Parameters params(1000, 1e-5f);
    mProjection.BuildLinearEquation();
    mLinearSolver.Solve(params);
    mProjection.ApplyPressure();

    mExtrapolation.Extrapolate();
    mExtrapolation.ConstrainVelocity();

    // 6)
    mParticleCount.TransferFromGrid();

    // 7)
    mAdvection.AdvectParticles();
    mClearVelocity.Submit();
    mClearValid.Submit();
}

Renderer::RenderTexture& World::Velocity()
{
    return mVelocity;
}

LevelSet& World::LiquidPhi()
{
    return mFluidLevelSet;
}

LevelSet& World::SolidPhi()
{
    return mObstacleLevelSet;
}

Renderer::GenericBuffer& World::Particles()
{
    return mParticles;
}

ParticleCount& World::Count()
{
    return mParticleCount;
}

Renderer::GenericBuffer& World::Valid()
{
    return mValid;
}

}}
