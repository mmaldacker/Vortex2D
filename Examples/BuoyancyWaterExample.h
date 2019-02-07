//
//  HydrostaticWaterExample.h
//  Vortex2D
//

#include <Vortex2D/Vortex2D.h>

#include "Runner.h"
#include "Rigidbody.h"

#include <functional>
#include <vector>
#include <memory>

extern glm::vec4 red;
extern glm::vec4 blue;

class HydrostaticWaterExample : public Runner
{
    const float gravityForce = 100.0f;

public:
    HydrostaticWaterExample(const Vortex2D::Renderer::Device& device,
                            const glm::ivec2& size,
                            float dt)
        : gravity(device, glm::vec2(256.0f, 256.0f))
        , world(device, size, dt, 2)
        , solidPhi(world.SolidDistanceField())
        , liquidPhi(world.LiquidDistanceField())
        , rWorld(b2Vec2(0.0f, gravityForce))
        , solver(rWorld)
        , circle1(device, size, rWorld, b2_dynamicBody, Vortex2D::Fluid::RigidBody::Type::eStrong, 10.0f, 0.4f)
        , circle2(device, size, rWorld, b2_dynamicBody, Vortex2D::Fluid::RigidBody::Type::eStrong, 10.0f, 0.7f)
        , circle3(device, size, rWorld, b2_dynamicBody, Vortex2D::Fluid::RigidBody::Type::eStrong, 10.0f, 1.1f)
        , left(device, size, rWorld, b2_staticBody, Vortex2D::Fluid::RigidBody::Type::eStatic, {5.0f, 125.0f})
        , right(device, size, rWorld, b2_staticBody, Vortex2D::Fluid::RigidBody::Type::eStatic, {5.0f, 125.0f})
        , bottom(device, size, rWorld, b2_staticBody, Vortex2D::Fluid::RigidBody::Type::eStatic, {250.0f, 5.0f})
    {
        world.AttachRigidBodySolver(solver);
        world.AddRigidbody(circle1.mRigidbody);
        world.AddRigidbody(circle2.mRigidbody);
        world.AddRigidbody(circle3.mRigidbody);
        world.AddRigidbody(left.mRigidbody);
        world.AddRigidbody(right.mRigidbody);
        world.AddRigidbody(bottom.mRigidbody);

        gravity.Colour = glm::vec4(0.0f, dt * gravityForce, 0.0f, 0.0f);

        solidPhi.Colour = red;
        liquidPhi.Colour = blue;
    }

    void Init(const Vortex2D::Renderer::Device& device,
              Vortex2D::Renderer::RenderTarget& renderTarget) override
    {
        // Add particles
        Vortex2D::Renderer::IntRectangle fluid(device, {225.0f, 75.0f});
        fluid.Position = {10.0f, 150.0f};
        fluid.Colour = glm::vec4(4);

        world.RecordParticleCount({fluid}).Submit().Wait();

        // Draw boundaries
        left.mRigidbody.SetTransform({3.0f, 250.0f}, 0.0f);
        right.mRigidbody.SetTransform({250.0f, 250.0f}, 0.0f);
        bottom.mRigidbody.SetTransform({3.0f, 250.0f}, 0.0f);

        // Add circles
        circle1.mRigidbody.SetTransform({50.0f, 100.0f}, 0.0f);
        circle2.mRigidbody.SetTransform({125.0f, 100.0f}, 0.0f);
        circle3.mRigidbody.SetTransform({200.0f, 100.0f}, 0.0f);

        // Set gravity
        velocityRender = world.RecordVelocity({gravity}, Vortex2D::Fluid::VelocityOp::Add);

        Vortex2D::Renderer::ColorBlendState blendState;
        blendState.ColorBlend
                .setBlendEnable(true)
                .setAlphaBlendOp(vk::BlendOp::eAdd)
                .setColorBlendOp(vk::BlendOp::eAdd)
                .setSrcColorBlendFactor(vk::BlendFactor::eSrcAlpha)
                .setSrcAlphaBlendFactor(vk::BlendFactor::eOne)
                .setDstColorBlendFactor(vk::BlendFactor::eOneMinusSrcAlpha)
                .setDstAlphaBlendFactor(vk::BlendFactor::eZero);

        windowRender = renderTarget.Record({liquidPhi, solidPhi}, blendState);
    }

    void Step() override
    {
        world.SubmitVelocity(velocityRender);
        auto params = Vortex2D::Fluid::FixedParams(12);
        world.Step(params);
        windowRender.Submit();
    }

private:
    Vortex2D::Renderer::Rectangle gravity;
    Vortex2D::Fluid::WaterWorld world;
    Vortex2D::Fluid::DistanceField solidPhi, liquidPhi;
    Vortex2D::Renderer::RenderCommand velocityRender, windowRender;

    b2World rWorld;

    Box2DSolver solver;
    CircleRigidbody circle1, circle2, circle3;
    RectangleRigidbody left, right, bottom;
};
