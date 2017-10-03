//
//  World.cpp
//  Vortex2D
//

#include "World.h"

#include <glm/gtx/transform.hpp>

namespace Vortex2D { namespace Fluid {

World::World(const Renderer::Device& device, Dimensions dimensions, float dt)
    : mDimensions(dimensions)
    , mParticles(device, vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eVertexBuffer, false, 8*dimensions.Size.x*dimensions.Size.y*sizeof(Particle))
    , mParticleCount(device, dimensions.Size, mParticles)
    , mPreconditioner(device, dimensions.Size)
    , mLinearSolver(device, dimensions.Size, mPreconditioner)
    , mData(device, dimensions.Size)
    , mVelocity(device, dimensions.Size.x, dimensions.Size.y, vk::Format::eR32G32Sfloat)
    , mBoundariesVelocity(device, dimensions.Size.x, dimensions.Size.y, vk::Format::eR32G32Sfloat)
    , mFluidLevelSet(device, dimensions.Size)
    , mObstacleLevelSet(device, dimensions.Size)
    , mValid(device, vk::BufferUsageFlagBits::eStorageBuffer, false, dimensions.Size.x*dimensions.Size.y*sizeof(glm::ivec2))
    , mAdvection(device, dimensions.Size, dt, mVelocity)
    , mProjection(device, dt, dimensions.Size,
                  mData,
                  mVelocity,
                  mObstacleLevelSet,
                  mFluidLevelSet,
                  mBoundariesVelocity,
                  mValid)
    , mExtrapolation(device, dimensions.Size, mValid, mVelocity, mObstacleLevelSet)
    , mClearVelocity(device, false)
{
    mParticleCount.InitLevelSet(mFluidLevelSet);
    mParticleCount.InitVelocities(mVelocity, mValid);
    mFluidLevelSet.ExtrapolateInit(mObstacleLevelSet);
    mAdvection.AdvectParticleInit(mParticles, mObstacleLevelSet, mParticleCount.GetDispatchParams());

    mClearVelocity.Record([&](vk::CommandBuffer commandBuffer)
    {
        mVelocity.Clear(commandBuffer, std::array<float, 4>{0.0f, 0.0f, 0.0f, 0.0f});
    });

    mLinearSolver.Init(mData.Diagonal, mData.Lower, mData.B, mData.X);
}

void World::InitField(Renderer::Texture& field)
{
    mAdvection.AdvectInit(field);
}

void World::SolveStatic()
{
    LinearSolver::Parameters params(300, 1e-3f);
    mProjection.BuildLinearEquation();
    mLinearSolver.Solve(params);
    mProjection.ApplyPressure();

    mExtrapolation.Extrapolate();
    mExtrapolation.ConstrainVelocity();

    mAdvection.AdvectVelocity();
    mAdvection.Advect();
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
    mParticleCount.Count();
    mClearVelocity;
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

Renderer::Buffer& World::Particles()
{
    return mParticles;
}

ParticleCount& World::Count()
{
    return mParticleCount;
}

}}
