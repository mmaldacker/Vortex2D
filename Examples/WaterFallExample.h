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
        : delta(dt / 2.0f)
        , waterSource(device, {10.0f, 10.0f})
        , waterForce(device, {10.0f, 10.0f})
        , gravity(device, glm::vec2(256.0f, 256.0f))
        , world(device, size, dt, 1)
        , solidPhi(world.SolidDistanceField())
        , liquidPhi(world.LiquidDistanceField())
        , rWorld(b2Vec2(0.0f, gravityForce))
        , circle(device, rWorld, b2_dynamicBody, world, Vortex2D::Fluid::RigidBody::Type::eStrong, 10.0f)
        , box(device, rWorld, b2_dynamicBody, world, Vortex2D::Fluid::RigidBody::Type::eStrong, {15.0f, 15.0f})
        , left(device, rWorld, b2_staticBody, world, Vortex2D::Fluid::RigidBody::Type::eStatic, {50.0f, 5.0f})
        , right(device, rWorld, b2_staticBody, world, Vortex2D::Fluid::RigidBody::Type::eStatic, {50.0f, 5.0f})
        , bottom(device, rWorld, b2_staticBody, world, Vortex2D::Fluid::RigidBody::Type::eStatic, {250.0f, 5.0f})
    {
        gravity.Colour = glm::vec4(0.0f, delta * gravityForce, 0.0f, 0.0f);

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
        left.SetTransform({50.0f, 80.0f}, 60.0f);
        left.Update();

        right.SetTransform({175.0f, 125.0f}, -60.0f);
        right.Update();

        bottom.SetTransform({5.0f, 250.0f}, 0.0f);
        bottom.Update();

        // Add circles
        circle.SetTransform({50.0f, 50.0f}, 0.0f);
        box.SetTransform({75.0f, 50.0f}, 0.0f);

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

    void Substep()
    {
        sourceRender.Submit();
        world.SubmitVelocity(velocityRender);
        auto params = Vortex2D::Fluid::FixedParams(12);
        world.Step(params);

        circle.Update();
        box.Update();

        const int velocityStep = 8;
        const int positionStep = 3;
        rWorld.Step(delta, velocityStep, positionStep);
    }

    void Step() override
    {
        Substep();
        Substep();
        windowRender.Submit();
    }

private:
    float delta;
    Vortex2D::Renderer::IntRectangle waterSource;
    Vortex2D::Renderer::Rectangle waterForce;
    Vortex2D::Renderer::Rectangle gravity;
    Vortex2D::Fluid::WaterWorld world;
    Vortex2D::Fluid::DistanceField solidPhi, liquidPhi;
    Vortex2D::Renderer::RenderCommand sourceRender, velocityRender, windowRender;

    b2World rWorld;

    CircleRigidbody circle;
    BoxRigidbody box;
    BoxRigidbody left, right, bottom;
};
