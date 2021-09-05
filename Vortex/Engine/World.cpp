//
//  World.cpp
//  Vortex
//

#include "World.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/transform.hpp>

namespace Vortex
{
namespace Fluid
{
template <typename Class>
void ForAll(std::vector<Class*>& elements, void (Class::*f)())
{
  for (auto& element : elements)
  {
    (*(element).*f)();
  }
}

uint32_t NextPowerOfTwo(uint32_t n)
{
  --n;

  n |= n >> 1;
  n |= n >> 2;
  n |= n >> 4;
  n |= n >> 8;
  n |= n >> 16;

  return n + 1;
}

glm::ivec2 NextPowerOfTwo(const glm::ivec2& s)
{
  return {NextPowerOfTwo(s.x), NextPowerOfTwo(s.y)};
}

World::World(const Renderer::Device& device,
             const glm::ivec2& size,
             float dt,
             int numSubSteps,
             Velocity::InterpolationMode interpolationMode)
    : mDevice(device)
    , mSize(size)
    , mDelta(dt / numSubSteps)
    , mNumSubSteps(numSubSteps)
    , mSolverSize(NextPowerOfTwo(size))
    , mPreconditioner(device, mSolverSize, mDelta)
    , mLinearSolver(device, mSolverSize, mPreconditioner)
    , mData(device, mSolverSize)
#if !defined(NDEBUG)
    , mDebugData(device, mSolverSize)
    , mDebugDataCopy(device, mSolverSize, mData, mDebugData)
#endif
    , mVelocity(device, size)
    , mLiquidPhi(device, size)
    , mStaticSolidPhi(device, size)
    , mDynamicSolidPhi(device, size)
    , mValid(device, size.x * size.y)
    , mAdvection(device, size, mDelta, mVelocity, interpolationMode)
    , mProjection(device,
                  mDelta,
                  mSolverSize,
                  mData,
                  mVelocity,
                  mDynamicSolidPhi,
                  mLiquidPhi,
                  mValid)
    , mExtrapolation(device, size, mValid, mVelocity)
    , mCopySolidPhi(device, false)
    , mRigidBodySolver(nullptr)
    , mCfl(device, size, mVelocity)
{
  mExtrapolation.ConstrainBind(mDynamicSolidPhi);
  mLiquidPhi.ExtrapolateBind(mDynamicSolidPhi);

  mCopySolidPhi.Record([&](vk::CommandBuffer commandBuffer)
                       { mDynamicSolidPhi.CopyFrom(commandBuffer, mStaticSolidPhi); });

  mPreconditioner.BuildHierarchiesBind(mProjection, mDynamicSolidPhi, mLiquidPhi);
  mLinearSolver.Bind(mData.Diagonal, mData.Lower, mData.B, mData.X);

  mDevice.Execute(
      [&](vk::CommandBuffer commandBuffer) {
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

Renderer::RenderCommand World::RecordVelocity(Renderer::RenderTarget::DrawableList drawables,
                                              VelocityOp op)
{
  Renderer::ColorBlendState blendState;
  blendState.ColorBlend.setBlendEnable(true)
      .setColorBlendOp(vk::BlendOp::eAdd)
      .setSrcColorBlendFactor(vk::BlendFactor::eConstantColor)
      .setDstColorBlendFactor(op == VelocityOp::Add ? vk::BlendFactor::eOne
                                                    : vk::BlendFactor::eZero);

  float scale = 1.0f / mSize.x;
  blendState.BlendConstants = {scale, scale, scale, scale};

  return mVelocity.Record(drawables, blendState);
}

void World::SubmitVelocity(Renderer::RenderCommand& renderCommand)
{
  mVelocities.push_back(&renderCommand);
}

Renderer::RenderCommand World::RecordLiquidPhi(Renderer::RenderTarget::DrawableList drawables)
{
  return mLiquidPhi.Record(drawables);
}

Renderer::RenderCommand World::RecordStaticSolidPhi(Renderer::RenderTarget::DrawableList drawables)
{
  return mStaticSolidPhi.Record(drawables, UnionBlend);
}

DistanceField World::LiquidDistanceField()
{
  return {mDevice, mLiquidPhi};
}

DistanceField World::SolidDistanceField()
{
  return {mDevice, mDynamicSolidPhi};
}

void World::AddRigidbody(RigidBody& rigidbody)
{
  rigidbody.BindPhi(mDynamicSolidPhi);
  rigidbody.BindDiv(mData.B, mData.Diagonal);
  rigidbody.BindVelocityConstrain(mVelocity);
  mLinearSolver.BindRigidbody(mDelta, mData.Diagonal, rigidbody);
  rigidbody.BindForce(mData.Diagonal, mData.X);

  mRigidbodies.push_back(&rigidbody);
}

void World::RemoveRigidBody(RigidBody& rigidbody)
{
  mRigidbodies.erase(std::remove(mRigidbodies.begin(), mRigidbodies.end(), &rigidbody),
                     mRigidbodies.end());
}

void World::AttachRigidBodySolver(RigidBodySolver& rigidbodySolver)
{
  mRigidBodySolver = &rigidbodySolver;
}

void World::StepRigidBodies()
{
  // Set Forces to rigid bodies
  ForAll(mRigidbodies, &RigidBody::ApplyForces);

  if (mRigidBodySolver)
  {
    mRigidBodySolver->Step(mDelta);
  }

  // Set Velocities to fluid rigid bodies
  ForAll(mRigidbodies, &RigidBody::ApplyVelocities);
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

SmokeWorld::SmokeWorld(const Renderer::Device& device,
                       const glm::ivec2& size,
                       float dt,
                       Velocity::InterpolationMode interpolationMode)
    : World(device, size, dt, 1, interpolationMode)
{
}

SmokeWorld::~SmokeWorld() {}

void SmokeWorld::Substep(LinearSolver::Parameters& params)
{
  for (auto& velocity : mVelocities)
  {
    velocity->Submit();
  }
  mVelocities.clear();

  mCopySolidPhi.Submit();

  ForAll(mRigidbodies, &RigidBody::RenderPhi);
  ForAll(mRigidbodies, &RigidBody::UpdatePosition);

  mDynamicSolidPhi.Reinitialise();
  mPreconditioner.BuildHierarchies();
  mProjection.BuildLinearEquation();

  ForAll(mRigidbodies, &RigidBody::Div);

  mLinearSolver.Solve(params, mRigidbodies);
  mProjection.ApplyPressure();

#if !defined(NDEBUG)
  mDebugDataCopy.Copy();
#endif

  ForAll(mRigidbodies, &RigidBody::Force);

  mExtrapolation.Extrapolate();
  mExtrapolation.ConstrainVelocity();

  ForAll(mRigidbodies, &RigidBody::VelocityConstrain);

  mAdvection.AdvectVelocity();
  mAdvection.Advect();

  StepRigidBodies();
}

void SmokeWorld::FieldBind(Density& density)
{
  mAdvection.AdvectBind(density);
}

WaterWorld::WaterWorld(const Renderer::Device& device,
                       const glm::ivec2& size,
                       float dt,
                       int numSubSteps,
                       Velocity::InterpolationMode interpolationMode)
    : World(device, size, dt, numSubSteps, interpolationMode)
    , mParticles(device,
                 vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eVertexBuffer,
                 VMA_MEMORY_USAGE_GPU_ONLY,
                 8 * size.x * size.y * sizeof(Particle))
    , mParticleCount(device, size, mParticles, interpolationMode, {0}, 0.02f)
{
  mParticleCount.LevelSetBind(mLiquidPhi);
  mParticleCount.VelocitiesBind(mVelocity, mValid);
  mAdvection.AdvectParticleBind(mParticles, mDynamicSolidPhi, mParticleCount.GetDispatchParams());
}

WaterWorld::~WaterWorld() {}

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
  ParticlePhi();

  // 2)
  mParticleCount.TransferToGrid();
  mExtrapolation.Extrapolate();
  mVelocity.SaveCopy();

  // 3)
  for (auto& velocity : mVelocities)
  {
    velocity->Submit();
  }
  mVelocities.clear();

  // 4)
  mCopySolidPhi.Submit();
  ForAll(mRigidbodies, &RigidBody::RenderPhi);
  ForAll(mRigidbodies, &RigidBody::UpdatePosition);
  mDynamicSolidPhi.Reinitialise();

  ForAll(mRigidbodies, &RigidBody::Div);

  mPreconditioner.BuildHierarchies();
  mLiquidPhi.Extrapolate();

  // 5)
  mProjection.BuildLinearEquation();
  mLinearSolver.Solve(params, mRigidbodies);
  mProjection.ApplyPressure();

#if !defined(NDEBUG)
  mDebugDataCopy.Copy();
#endif

  ForAll(mRigidbodies, &RigidBody::Force);

  mExtrapolation.Extrapolate();
  mExtrapolation.ConstrainVelocity();

  ForAll(mRigidbodies, &RigidBody::VelocityConstrain);

  // 6)
  mVelocity.VelocityDiff();
  mParticleCount.TransferFromGrid();

  // 7)
  mAdvection.AdvectParticles();

  // 8)
  StepRigidBodies();
}

Renderer::RenderCommand WaterWorld::RecordParticleCount(
    Renderer::RenderTarget::DrawableList drawables)
{
  return mParticleCount.Record(drawables);
}

void WaterWorld::ParticlePhi()
{
  mParticleCount.Scan();
  mParticleCount.Phi();
  mLiquidPhi.Reinitialise();
}

}  // namespace Fluid
}  // namespace Vortex
