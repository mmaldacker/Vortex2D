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
                           Renderer::Drawable& drawable,
                           const glm::vec2& centre,
                           Renderer::RenderTexture& phi,
                           vk::Flags<Type> type);

    VORTEX2D_API void SetVelocities(const glm::vec2& velocity, float angularVelocity);

    VORTEX2D_API void UpdatePosition();
    VORTEX2D_API void RenderPhi();

    VORTEX2D_API void BindDiv(Renderer::GenericBuffer& div,
                              Renderer::GenericBuffer& diagonal,
                              Renderer::Texture& fluidLevelSet);

    VORTEX2D_API void BindVelocityConstrain(Fluid::Velocity& velocity);
    VORTEX2D_API void BindPressure(Renderer::Texture& fluidLevelSet,
                                   Renderer::GenericBuffer& pressure,
                                   Renderer::GenericBuffer& force);

    VORTEX2D_API void Div();
    VORTEX2D_API void Pressure();
    VORTEX2D_API void VelocityConstrain();
    VORTEX2D_API Velocity GetForces();
    VORTEX2D_API vk::Flags<Type> GetType();
    VORTEX2D_API Renderer::RenderTexture& Phi();

private:
    float mScale;
    const Renderer::Device& mDevice;
    Renderer::RenderTexture mPhi;
    Renderer::Drawable& mDrawable;
    glm::vec2 mCentre;
    glm::mat4 mView;
    Renderer::UniformBuffer<Velocity> mVelocity;
    Renderer::Buffer<Velocity> mForce;
    Renderer::UniformBuffer<glm::mat4> mMVBuffer;

    Renderer::Clear mClear;
    Renderer::RenderCommand mLocalPhiRender, mPhiRender;

    Renderer::Work mDiv, mConstrain, mPressure;
    Renderer::Work::Bound mDivBound, mConstrainBound, mPressureBound;
    Renderer::CommandBuffer mDivCmd, mConstrainCmd, mPressureCmd;
    ReduceJ mSum;
    ReduceSum::Bound mSumBound;

    vk::Flags<Type> mType;
};

}}

#endif
