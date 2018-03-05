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

    RigidBody(const Renderer::Device& device,
              const Dimensions& dimensions,
              ObjectDrawable& drawable,
              const glm::vec2& centre,
              Renderer::RenderTexture& phi,
              vk::Flags<Type> type);

    void SetVelocities(const glm::vec2& velocity, float angularVelocity);

    void UpdatePosition();
    void RenderPhi();

    void BindDiv(Renderer::GenericBuffer& div,
                 Renderer::GenericBuffer& diagonal,
                 Renderer::Texture& fluidLevelSet);

    void BindVelocityConstrain(Fluid::Velocity& velocity);
    void BindPressure(Renderer::Texture& fluidLevelSet,
                      Renderer::GenericBuffer& pressure,
                      Renderer::GenericBuffer& force);

    void Div();
    void Pressure();
    void VelocityConstrain();
    Velocity GetForces();
    vk::Flags<Type> GetType();
    Renderer::RenderTexture& Phi();

private:
    const Renderer::Device& mDevice;
    Renderer::RenderTexture mPhi;
    ObjectDrawable& mDrawable;
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
