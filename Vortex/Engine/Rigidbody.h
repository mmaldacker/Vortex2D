//
//  Rigidbody.h
//  Vortex
//

#pragma once

#include <Vortex/Engine/Boundaries.h>
#include <Vortex/Engine/LinearSolver/Reduce.h>
#include <Vortex/Engine/Velocity.h>
#include <Vortex/Renderer/Buffer.h>
#include <Vortex/Renderer/Drawable.h>
#include <Vortex/Renderer/Pipeline.h>
#include <Vortex/Renderer/RenderTexture.h>
#include <Vortex/Renderer/Shapes.h>
#include <Vortex/Renderer/Transformable.h>
#include <Vortex/Renderer/Work.h>

namespace Vortex
{
namespace Fluid
{
/**
 * @brief Interface to call the external rigidbody solver
 */
class RigidBodySolver
{
public:
  /**
   * @brief perfoms a single step of the solver.
   * @param delta of simulation
   */
  virtual void Step(float delta) = 0;
};

/**
 * @brief Rigidbody that can interact with the fluid: either be push by it, or
 * influence it, or both.
 */
class RigidBody : public Renderer::Transformable
{
public:
  enum class Type
  {
    eStatic,
    eWeak,
    eStrong,
  };

  struct Velocity
  {
    alignas(8) glm::vec2 velocity;
    alignas(4) float angular_velocity;
  };

  VORTEX_API RigidBody(Renderer::Device& device,
                       const glm::ivec2& size,
                       Renderer::DrawablePtr drawable,
                       Type type);

  VORTEX_API ~RigidBody();

  /**
   * @brief function to override and apply forces from this rigidbody to the
   * external rigidbody
   */
  VORTEX_API virtual void ApplyForces();

  /**
   * @brief Override and apply velocities from the external rigidbody
   * to the this rigidbody
   */
  VORTEX_API virtual void ApplyVelocities();

  /**
   * @brief Sets the mass and inertia of the rigidbody
   * @param mass of the body
   * @param inertia of the body
   */
  VORTEX_API void SetMassData(float mass, float inertia);

  /**
   * @brief sets the velocities and angular velocities of the body
   * @param velocity
   * @param angularVelocity
   */
  VORTEX_API void SetVelocities(const glm::vec2& velocity, float angularVelocity);

  /**
   * @brief Upload the transform matrix to the GPU.
   */
  VORTEX_API void UpdatePosition();

  /**
   * @brief Render the current object orientation in an internal texture and the
   * external one.
   */
  VORTEX_API void RenderPhi();

  /**
   * @brief Bind the rendertexture where this rigidbodies shape will be rendered
   * @param phi render texture of the world
   */
  VORTEX_API void BindPhi(Renderer::RenderTexture& phi);

  /**
   * @brief Bind a the right hand side and diagonal of the linear system Ax = b.
   * This is to apply the rigid body influence to the system.
   * @param div right hand side of the linear system Ax=b
   * @param diagonal diagonal of matrix A
   */
  VORTEX_API void BindDiv(Renderer::GenericBuffer& div, Renderer::GenericBuffer& diagonal);

  /**
   * @brief Bind velocities to constrain based on the body's velocity.
   * @param velocity
   */
  VORTEX_API void BindVelocityConstrain(Fluid::Velocity& velocity);

  /**
   * @brief Bind pressure, to have the pressure update the body's forces
   * @param d diagonal of matrix A
   * @param pressure solved pressure buffer
   */
  VORTEX_API void BindForce(Renderer::GenericBuffer& d, Renderer::GenericBuffer& pressure);

  /**
   * @brief Bind pressure, to have the pressure update the body's forces
   * @param delta
   * @param d
   * @param s
   * @param z
   */
  VORTEX_API void BindPressure(float delta,
                               Renderer::GenericBuffer& d,
                               Renderer::GenericBuffer& s,
                               Renderer::GenericBuffer& z);

  /**
   * @brief Apply the body's velocities to the linear equations matrix A and
   * right hand side b.
   */
  VORTEX_API void Div();

  /**
   * @brief Apply the pressure to body, updating its forces.
   */
  VORTEX_API void Force();

  /**
   * @brief Reduce the force for pressure update.
   */
  VORTEX_API void Pressure();

  /**
   * @brief Constrain the velocities field based on the body's velocity.
   */
  VORTEX_API void VelocityConstrain();

  /**
   * @brief Download the forces from the GPU and return them.
   * @return
   */
  VORTEX_API Velocity GetForces();

  /**
   * @brief Type of this body.
   * @return
   */
  VORTEX_API Type GetType();

  /**
   * @brief Set the type of the body.
   * @param type
   */
  VORTEX_API void SetType(Type type);

  /**
   * @brief the local level set of the body
   * @return
   */
  VORTEX_API Renderer::RenderTexture& Phi();

private:
  float mSize;

  Renderer::Device& mDevice;
  Renderer::DrawablePtr mDrawable;
  Renderer::RenderTexture mPhi;
  Renderer::UniformBuffer<Velocity> mVelocity;
  Renderer::Buffer<Velocity> mForce, mReducedForce, mLocalForce;
  Renderer::UniformBuffer<glm::vec2> mCenter;
  Renderer::UniformBuffer<Velocity> mLocalVelocity;

  Renderer::RenderCommand mLocalPhiRender, mPhiRender;

  Renderer::Work mDiv, mConstrain, mForceWork, mPressureWork;
  Renderer::Work::Bound mDivBound, mConstrainBound, mForceBound, mPressureForceBound,
      mPressureBound;
  Renderer::CommandBuffer mDivCmd, mConstrainCmd, mForceCmd, mPressureCmd, mVelocityCmd;
  ReduceJ mSum;
  ReduceSum::Bound mLocalSumBound, mSumBound;

  Type mType;
  float mMass;
  float mInertia;
};

}  // namespace Fluid
}  // namespace Vortex
