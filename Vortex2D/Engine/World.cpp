//
//  World.cpp
//  Vortex2D
//

#include "World.h"

#include <glm/gtx/transform.hpp>

namespace Vortex2D { namespace Fluid {

World::World(const Renderer::Device& device, Dimensions dimensions, float dt)
    : mDimensions(dimensions)
    , mDiagonal(device, vk::BufferUsageFlagBits::eStorageBuffer, false, dimensions.Size.x*dimensions.Size.y*sizeof(float))
    , mLower(device, vk::BufferUsageFlagBits::eStorageBuffer, false, dimensions.Size.x*dimensions.Size.y*sizeof(glm::vec2))
    , mDiv(device, vk::BufferUsageFlagBits::eStorageBuffer, false, dimensions.Size.x*dimensions.Size.y*sizeof(float))
    , mPressure(device, vk::BufferUsageFlagBits::eStorageBuffer, false, dimensions.Size.x*dimensions.Size.y*sizeof(float))
    , mParticles(device, vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eVertexBuffer, false, 8*dimensions.Size.x*dimensions.Size.y*sizeof(Particle))
    , mParticleCount(device, dimensions.Size, mParticles)
    , mPreconditioner(device, dimensions.Size)
    , mLinearSolver(device, dimensions.Size, mPreconditioner)
    , mVelocity(device, dimensions.Size.x, dimensions.Size.y, vk::Format::eR32G32Sfloat)
    , mBoundariesVelocity(device, dimensions.Size.x, dimensions.Size.y, vk::Format::eR32G32Sfloat)
    , mFluidLevelSet(device, dimensions.Size)
    , mObstacleLevelSet(device, dimensions.Size)
    , mValid(device, vk::BufferUsageFlagBits::eStorageBuffer, false, dimensions.Size.x*dimensions.Size.y*sizeof(glm::ivec2))
    , mAdvection(device, dimensions.Size, dt, mVelocity)
    , mProjection(device, dt, dimensions.Size, mLinearSolver, mVelocity, mObstacleLevelSet, mFluidLevelSet, mBoundariesVelocity)
    , mExtrapolation(device, dimensions.Size, mValid, mVelocity, mObstacleLevelSet)
{
    mPreconditioner.SetW(1.5f);
    mPreconditioner.SetPreconditionerIterations(16);
}

void World::InitField(Renderer::Texture& field)
{
    mAdvection.AdvectInit(field);
}

void World::SolveStatic()
{
    LinearSolver::Parameters params(300, 1e-3f);
    mProjection.Solve(params);

    mExtrapolation.Extrapolate();
    mExtrapolation.ConstrainVelocity();

    mAdvection.AdvectVelocity();
    mAdvection.Advect();
}

void World::SolveDynamic()
{

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
