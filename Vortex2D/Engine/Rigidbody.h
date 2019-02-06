//
//  Rigidbody.h
//  Vortex2D
//

#ifndef Vortex2d_Rigidbody_h
#define Vortex2d_Rigidbody_h

#include <Vortex2D/Renderer/Buffer.h>
#include <Vortex2D/Renderer/Drawable.h>
#include <Vortex2D/Renderer/Transformable.h>
#include <Vortex2D/Renderer/Pipeline.h>
#include <Vortex2D/Renderer/RenderTexture.h>
#include <Vortex2D/Renderer/Work.h>
#include <Vortex2D/Renderer/Shapes.h>
#include <Vortex2D/Engine/LinearSolver/Reduce.h>
#include <Vortex2D/Engine/Boundaries.h>
#include <Vortex2D/Engine/Velocity.h>

namespace Vortex2D { namespace Fluid {

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
 * @brief Rigidbody that can interact with the fluid: either be push by it, or influence it, or both.
 */
class RigidBody : public Renderer::Transformable
{
public:
    enum class Type
    {
        eStatic = 0x01,
        eWeak = 0x02,
        eStrong = 0x3,
    };

    struct Velocity
    {
        alignas(8) glm::vec2 velocity;
        alignas(4) float angular_velocity;
    };

    VORTEX2D_API RigidBody(const Renderer::Device& device,
                           const glm::ivec2& size,
                           Renderer::Drawable& drawable,
                           vk::Flags<Type> type);

    VORTEX2D_API ~RigidBody();

    /**
     * @brief function to override and apply forces from this rigidbody to the external rigidbody
     */
    VORTEX2D_API virtual void ApplyForces();

    /**
     * @brieffunction to override and apply velocities from the external rigidbody to the this rigidbody
     */
    VORTEX2D_API virtual void ApplyVelocities();

    /**
     * @brief Sets the mass and inertia of the rigidbody
     * @param mass of the body
     * @param inertia of the body
     */
    VORTEX2D_API void SetMassData(float mass, float inertia);

    /**
     * @brief sets the velocities and angular velocities of the body
     * @param velocity
     * @param angularVelocity
     */
    VORTEX2D_API void SetVelocities(const glm::vec2& velocity, float angularVelocity);

    /**
     * @brief Upload the transform matrix to the GPU.
     */
    VORTEX2D_API void UpdatePosition();

    /**
     * @brief Render the current object orientation in an internal texture and the external one.
     */
    VORTEX2D_API void RenderPhi();

    /**
     * @brief Bind the rendertexture where this rigidbodies shape will be rendered
     * @param phi render texture of the world
     */
    VORTEX2D_API void BindPhi(Renderer::RenderTexture& phi);

    /**
     * @brief Bind a the right hand side and diagonal of the linear system Ax = b.
     * This is to apply the rigid body influence to the system.
     * @param div right hand side of the linear system Ax=b
     * @param diagonal diagonal of matrix A
     * @param fluidLevelSet fluid level set
     */
    VORTEX2D_API void BindDiv(Renderer::GenericBuffer& div,
                              Renderer::GenericBuffer& diagonal);

    /**
     * @brief Bind velocities to constrain based on the body's velocity.
     * @param velocity
     */
    VORTEX2D_API void BindVelocityConstrain(Fluid::Velocity& velocity);

    /**
     * @brief Bind pressure, to have the pressure update the body's forces
     * @param fluidLevelSet fluid level set, to know if the pressure is applicable
     * @param pressure solved pressure buffer
     */
    VORTEX2D_API void BindForce(Renderer::GenericBuffer& d,
                                Renderer::GenericBuffer& pressure);

    /**
     * @brief Bind pressure, to have the pressure update the body's forces
     * @param delta
     * @param d
     * @param s
     * @param z
     */
    VORTEX2D_API void BindPressure(float delta,
                                   Renderer::GenericBuffer& d,
                                   Renderer::GenericBuffer& s,
                                   Renderer::GenericBuffer& z);

    /**
     * @brief Apply the body's velocities to the linear equations matrix A and right hand side b.
     */
    VORTEX2D_API void Div();

    /**
     * @brief Apply the pressure to body, updating its forces.
     */
    VORTEX2D_API void Force();

    /**
     * @brief Reduce the force for pressure update.
     */
    VORTEX2D_API void Pressure();

    /**
     * @brief Constrain the velocities field based on the body's velocity.
     */
    VORTEX2D_API void VelocityConstrain();

    /**
     * @brief Download the forces from the GPU and return them.
     * @return
     */
    VORTEX2D_API Velocity GetForces();

    /**
     * @brief Type of this body.
     * @return
     */
    VORTEX2D_API vk::Flags<Type> GetType();

    /**
     * @brief the local level set of the body
     * @return
     */
    VORTEX2D_API Renderer::RenderTexture& Phi();

private:
    float mSize;

    const Renderer::Device& mDevice;
    Renderer::Drawable& mDrawable;
    Renderer::RenderTexture mPhi;
    Renderer::UniformBuffer<Velocity> mVelocity;
    Renderer::Buffer<Velocity> mForce, mReducedForce, mLocalForce;
    Renderer::UniformBuffer<glm::vec2> mCenter;
    Renderer::UniformBuffer<Velocity> mLocalVelocity;

    Renderer::Clear mClear;
    Renderer::RenderCommand mLocalPhiRender, mPhiRender;

    Renderer::Work mDiv, mConstrain, mForceWork, mPressureWork;
    Renderer::Work::Bound mDivBound, mConstrainBound, mForceBound, mPressureForceBound, mPressureBound;
    Renderer::CommandBuffer mDivCmd, mConstrainCmd, mForceCmd, mPressureCmd, mVelocityCmd;
    ReduceJ mSum;
    ReduceSum::Bound mLocalSumBound, mSumBound;

    vk::Flags<Type> mType;
    float mMass;
    float mInertia;
};

}}

#endif
