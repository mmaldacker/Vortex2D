//
//  WaterFallExample.h
//  Vortex2D
//

#include <Vortex2D/Vortex2D.h>

#include "Runner.h"
#include "Rigidbody.h"

#include <functional>
#include <vector>
#include <memory>

extern glm::vec4 green;
extern glm::vec4 blue;

class WaterFallExample : public Runner
{
    const float gravityForce = 100.0f;

public:
    WaterFallExample(const Vortex2D::Renderer::Device& device,
                     const glm::ivec2& size,
                     float dt)
        : waterSource(device, {10.0f, 10.0f})
        , waterForce(device, {10.0f, 10.0f})
        , gravity(device, glm::vec2(256.0f, 256.0f))
        , world(device, size, dt, 2)
        , solidPhi(world.SolidDistanceField())
        , liquidPhi(world.LiquidDistanceField())
        , rWorld(b2Vec2(0.0f, gravityForce))
        , solver(rWorld)
        , circle(device, size, rWorld, b2_dynamicBody, Vortex2D::Fluid::RigidBody::Type::eStrong, 10.0f)
        , box(device, size, rWorld, b2_dynamicBody, Vortex2D::Fluid::RigidBody::Type::eStrong, {15.0f, 15.0f})
        , left(device, size, rWorld, b2_staticBody, Vortex2D::Fluid::RigidBody::Type::eStatic, {50.0f, 5.0f})
        , right(device, size, rWorld, b2_staticBody, Vortex2D::Fluid::RigidBody::Type::eStatic, {50.0f, 5.0f})
        , bottom(device, size, rWorld, b2_staticBody, Vortex2D::Fluid::RigidBody::Type::eStatic, {250.0f, 5.0f})
    {
        world.AttachRigidBodySolver(solver);
        world.AddRigidbody(circle.mRigidbody);
        world.AddRigidbody(box.mRigidbody);
        world.AddRigidbody(left.mRigidbody);
        world.AddRigidbody(right.mRigidbody);
        world.AddRigidbody(bottom.mRigidbody);

        gravity.Colour = glm::vec4(0.0f, dt * gravityForce, 0.0f, 0.0f);

        solidPhi.Colour = green;
        liquidPhi.Colour = blue;
    }

    void Init(const Vortex2D::Renderer::Device& device,
              Vortex2D::Renderer::RenderTarget& renderTarget) override
    {

        // Add particles
        waterSource.Position = {5.0f, 25.0f};
        waterSource.Colour = glm::vec4(4);

        // Add force
        waterForce.Position = {5.0f, 25.0f};
        waterForce.Colour = glm::vec4(10.0f, 0.0f, 0.0f, 0.0f);

        sourceRender = world.RecordParticleCount({waterSource});

        // Draw boundaries
        left.mRigidbody.SetTransform({50.0f, 80.0f}, 20.0f);
        right.mRigidbody.SetTransform({175.0f, 125.0f}, -20.0f);
        bottom.mRigidbody.SetTransform({5.0f, 250.0f}, 0.0f);

        // Add circles
        circle.mRigidbody.SetTransform({50.0f, 50.0f}, 0.0f);
        box.mRigidbody.SetTransform({75.0f, 50.0f}, 0.0f);

        // Set gravity
        velocityRender = world.RecordVelocity({gravity, waterForce});

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
        sourceRender.Submit();
        world.SubmitVelocity(velocityRender);
        auto params = Vortex2D::Fluid::FixedParams(12);
        world.Step(params);
        windowRender.Submit();
    }

private:
    Vortex2D::Renderer::IntRectangle waterSource;
    Vortex2D::Renderer::Rectangle waterForce;
    Vortex2D::Renderer::Rectangle gravity;
    Vortex2D::Fluid::WaterWorld world;
    Vortex2D::Fluid::DistanceField solidPhi, liquidPhi;
    Vortex2D::Renderer::RenderCommand sourceRender, velocityRender, windowRender;

    b2World rWorld;

    Box2DSolver solver;
    CircleRigidbody circle;
    RectangleRigidbody box;
    RectangleRigidbody left, right, bottom;
};
