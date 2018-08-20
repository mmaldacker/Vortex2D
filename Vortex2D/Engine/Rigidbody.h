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
#include <Vortex2D/Engine/Size.h>
#include <Vortex2D/Engine/Velocity.h>

namespace Vortex2D { namespace Fluid {

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
                           const Dimensions& dimensions,
                           float dt,
                           Renderer::Drawable& drawable,
                           const glm::vec2& centre,
                           Renderer::RenderTexture& phi,
                           vk::Flags<Type> type,
                           float mass,
                           float inertia);

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
     * @brief Bind a the right hand side and diagonal of the linear system Ax = b.
     * This is to apply the rigid body influence to the system.
     * @param div right hand side of the linear system Ax=b
     * @param diagonal diagonal of matrix A
     * @param fluidLevelSet fluid level set
     */
    VORTEX2D_API void BindDiv(Renderer::GenericBuffer& div,
                              Renderer::GenericBuffer& diagonal,
                              Renderer::Texture& fluidLevelSet);

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
    VORTEX2D_API void BindForce(Renderer::Texture& fluidLevelSet,
                                Renderer::GenericBuffer& pressure);

    /**
     * @brief Bind pressure, to have the pressure update the body's forces
     * @param fluidLevelSet fluid level set, to know if the pressure is applicable
     * @param pressure solved pressure buffer
     */
    VORTEX2D_API void BindPressure(Renderer::Texture& fluidLevelSet,
                                   Renderer::GenericBuffer& pressure);


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
    float mScale;
    float mDelta;

    const Renderer::Device& mDevice;
    Renderer::RenderTexture mPhi;
    Renderer::Drawable& mDrawable;
    glm::vec2 mCentre;
    glm::mat4 mView;
    Renderer::UniformBuffer<Velocity> mVelocity;
    Renderer::Buffer<Velocity> mForce, mReducedForce, mLocalForce;
    Renderer::UniformBuffer<glm::mat4> mMVBuffer;

    Renderer::Clear mClear;
    Renderer::RenderCommand mLocalPhiRender, mPhiRender;

    Renderer::Work mDiv, mConstrain, mForceWork, mTansWork, mPressureWork;
    Renderer::Work::Bound mDivBound, mConstrainBound, mForceBound, mTransBound, mPressureBound;
    Renderer::CommandBuffer mDivCmd, mConstrainCmd, mForceCmd, mPressureCmd;
    ReduceJ mSum;
    ReduceSum::Bound mLocalSumBound, mSumBound;

    vk::Flags<Type> mType;
    float mMass;
    float mInertia;
};

}}

#endif
