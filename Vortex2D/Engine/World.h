//
//  World.h
//  Vortex2D
//

#ifndef Vortex2D_Engine_h
#define Vortex2D_Engine_h

#include <Vortex2D/Renderer/CommandBuffer.h>
#include <Vortex2D/Renderer/Drawable.h>
#include <Vortex2D/Renderer/Shapes.h>

#include <Vortex2D/Engine/Advection.h>
#include <Vortex2D/Engine/Boundaries.h>
#include <Vortex2D/Engine/Cfl.h>
#include <Vortex2D/Engine/Density.h>
#include <Vortex2D/Engine/Extrapolation.h>
#include <Vortex2D/Engine/LevelSet.h>
#include <Vortex2D/Engine/LinearSolver/ConjugateGradient.h>
#include <Vortex2D/Engine/LinearSolver/LinearSolver.h>
#include <Vortex2D/Engine/LinearSolver/Multigrid.h>
#include <Vortex2D/Engine/Particles.h>
#include <Vortex2D/Engine/Pressure.h>
#include <Vortex2D/Engine/Rigidbody.h>
#include <Vortex2D/Engine/Velocity.h>

#include <functional>
#include <memory>
#include <vector>

namespace Vortex2D
{
namespace Fluid
{
/**
 * @brief Operator when applying velocity to velocity field: add or set.
 */
enum class VelocityOp
{
  Add,
  Set
};

/**
 * @brief The main class of the framework. Each instance manages a grid and this
 * class is used to set forces, define boundaries, solve the incompressbility
 * equations and do the advection.
 */
class World
{
public:
  /**
   * @brief Construct an Engine with a size and time step.
   * @param device vulkan device
   * @param size dimensions of the simulation
   * @param dt timestamp of the simulation, e.g. 0.016 for 60FPS simulations.
   * @param numSubSteps the number of sub-steps to perform per step call.
   * Reduces loss of fluid.
   */
  World(const Renderer::Device& device,
        const glm::ivec2& size,
        float dt,
        int numSubSteps = 1,
        Velocity::InterpolationMode interpolationMode = Velocity::InterpolationMode::Linear);
  virtual ~World() = default;

  /**
   * @brief Perform one step of the simulation.
   */
  VORTEX2D_API void Step(LinearSolver::Parameters& params);

  /**
   * @brief Record drawables to the velocity field. The colour (r,g) will be
   * used as the velocity (x, y)
   * @param drawables a list of drawable field
   * @param op operation of the render: add velocity or set velocity
   * @return render command
   */
  VORTEX2D_API Renderer::RenderCommand RecordVelocity(
      Renderer::RenderTarget::DrawableList drawables,
      VelocityOp op);

  /**
   * @brief submit the render command created with @ref RecordVelocity
   * @param renderCommand the render command
   */
  VORTEX2D_API void SubmitVelocity(Renderer::RenderCommand& renderCommand);

  /**
   * @brief Record drawables to the liquid level set, i.e. to define the fluid
   * area. The drawables need to make a signed distance field, if not the result
   * is undefined.
   * @param drawables a list of signed distance field drawables
   * @return render command
   */
  VORTEX2D_API Renderer::RenderCommand RecordLiquidPhi(
      Renderer::RenderTarget::DrawableList drawables);

  /**
   * @brief Record drawables to the solid level set, i.e. to define the boundary
   * area. The drawables need to make a signed distance field, if not the result
   * is undefined.
   * @param drawables a list of signed distance field drawables
   * @return render command
   */
  VORTEX2D_API Renderer::RenderCommand RecordStaticSolidPhi(
      Renderer::RenderTarget::DrawableList drawables);

  /**
   * @brief Create sprite that can be rendered to visualize the liquid level
   * set.
   * @return a sprite
   */
  VORTEX2D_API DistanceField LiquidDistanceField();

  /**
   * @brief Create sprite that can be rendered to visualize the solid level set.
   * @return a sprite
   */
  VORTEX2D_API DistanceField SolidDistanceField();

  /**
   * @brief Add a rigibody to the solver
   * @param rigidbody
   */
  VORTEX2D_API void AddRigidbody(RigidBody& rigidbody);

  /**
   * @brief Remove a rigidbody from the solver
   * @param rigidbody
   */
  VORTEX2D_API void RemoveRigidBody(RigidBody& rigidbody);

  /**
   * @brief Attach a rigidbody solver, e.g. box2d
   * @param rigidbodySolver
   */
  VORTEX2D_API void AttachRigidBodySolver(RigidBodySolver& rigidbodySolver);

  /**
   * @brief Calculate the CFL number, i.e. the width divided by the max velocity
   * @return CFL number
   */
  VORTEX2D_API float GetCFL();

  /**
   * @brief Get the velocity, can be used to display it.
   * @return velocity field reference
   */
  VORTEX2D_API Renderer::Texture& GetVelocity();

protected:
  void StepRigidBodies();
  virtual void Substep(LinearSolver::Parameters& params) = 0;

  const Renderer::Device& mDevice;
  glm::ivec2 mSize;
  float mDelta;
  int mNumSubSteps;

  Multigrid mPreconditioner;
  ConjugateGradient mLinearSolver;

  LinearSolver::Data mData;

#if !defined(NDEBUG)
  LinearSolver::DebugData mDebugData;
  LinearSolver::DebugCopy mDebugDataCopy;
#endif

  Fluid::Velocity mVelocity;
  LevelSet mLiquidPhi;
  LevelSet mStaticSolidPhi;
  LevelSet mDynamicSolidPhi;

  Renderer::Buffer<glm::ivec2> mValid;

  Advection mAdvection;
  Pressure mProjection;
  Extrapolation mExtrapolation;

  Renderer::CommandBuffer mCopySolidPhi;

  std::vector<RigidBody*> mRigidbodies;
  RigidBodySolver* mRigidBodySolver;
  std::vector<Renderer::RenderCommand*> mVelocities;

  Cfl mCfl;
};

/**
 * @brief A concrete implementation of @ref World to simulate 'smoke', or more
 * accurately dye in a liquid. The liquid cannot change location or size.
 */
class SmokeWorld : public World
{
public:
  VORTEX2D_API SmokeWorld(const Renderer::Device& device,
                          const glm::ivec2& size,
                          float dt,
                          Velocity::InterpolationMode interpolationMode);
  VORTEX2D_API ~SmokeWorld() override;

  /**
   * @brief Bind a density field to be moved around with the fluid
   * @param density the density field
   */
  VORTEX2D_API void FieldBind(Density& density);

private:
  void Substep(LinearSolver::Parameters& params) override;
};

/**
 * @brief A concrete implementation of @ref World to simulate water.
 */
class WaterWorld : public World
{
public:
  VORTEX2D_API WaterWorld(const Renderer::Device& device,
                          const glm::ivec2& size,
                          float dt,
                          int numSubSteps,
                          Velocity::InterpolationMode interpolationMode);
  VORTEX2D_API ~WaterWorld() override;

  /**
   * @brief The water simulation uses particles to define the water area.
   * In fact, the level set is built from the particles. This means to be able
   * to set an area, we can't use @ref RecordLiquidPhi. To define the particle
   * area, simply draw a regular shape. The colour r is used to determine if we
   * add or remove particles, use r = 4 to add and r = -4 to remove.
   * @param drawables list of drawables object with colour 8 or -8
   * @return render command
   */
  VORTEX2D_API Renderer::RenderCommand RecordParticleCount(
      Renderer::RenderTarget::DrawableList drawables);

  /**
   * @brief Using the particles, create a level set (phi) encompassing all the particles.
   * This can be viewed with @ref LiquidDistanceField
   */
  VORTEX2D_API void ParticlePhi();

private:
  void Substep(LinearSolver::Parameters& params) override;

  Renderer::GenericBuffer mParticles;
  ParticleCount mParticleCount;
};

}  // namespace Fluid
}  // namespace Vortex2D

#endif
