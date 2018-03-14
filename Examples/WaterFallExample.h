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

extern glm::vec4 red;
extern glm::vec4 blue;

class WaterFallExample : public Runner
{
    const float gravityForce = 300.0f;

public:
    WaterFallExample(const Vortex2D::Renderer::Device& device,
                     const Vortex2D::Fluid::Dimensions& dimensions,
                     float dt)
        : delta(dt)
        , waterSource(device, {50.0f, 50.0f})
        , waterForce(device, {50.0f, 50.0f})
        , gravity(device, glm::vec2(1024.0f, 1024.0f))
        , world(device, dimensions, dt)
        , solidPhi(world.SolidDistanceField())
        , liquidPhi(world.LiquidDistanceField())
        , rWorld(b2Vec2(0.0f, gravityForce / box2dScale))
        , circle(device, dimensions, rWorld, b2_dynamicBody, world, Vortex2D::Fluid::RigidBody::Type::eStrong, 40.0f)
        , box(device, dimensions, rWorld, b2_dynamicBody, world, Vortex2D::Fluid::RigidBody::Type::eStrong, {50.0f, 50.0f})
        , left(device, dimensions, rWorld, b2_staticBody, world, Vortex2D::Fluid::RigidBody::Type::eStatic, {200.0f, 20.0f})
        , right(device, dimensions, rWorld, b2_staticBody, world, Vortex2D::Fluid::RigidBody::Type::eStatic, {200.0f, 20.0f})
        , bottom(device, dimensions, rWorld, b2_staticBody, world, Vortex2D::Fluid::RigidBody::Type::eStatic, {1000.0f, 20.0f})
    {
        liquidPhi.Scale = solidPhi.Scale = glm::vec2(dimensions.Scale);
        gravity.Colour = glm::vec4(0.0f, dt * gravityForce / (dimensions.Scale * dimensions.Size.x), 0.0f, 0.0f);

        solidPhi.Colour = red;
        liquidPhi.Colour = blue;
    }

    void Init(const Vortex2D::Renderer::Device& device,
              Vortex2D::Renderer::RenderTarget& renderTarget) override
    {

        // Add particles
        waterSource.Position = {20.0f, 100.0f};
        waterSource.Colour = glm::vec4(4);

        // Add force
        waterForce.Position = {20.0f, 100.0f};
        waterForce.Colour = glm::vec4(0.01f, 0.0f, 0.0f, 0.0f);

        sourceRender = world.RecordParticleCount({waterSource});

        // Draw boundaries
        left.SetTransform({200.0f, 350.0f}, 60.0f);
        left.Update();

        right.SetTransform({700.0f, 500.0f}, -60.0f);
        right.Update();

        bottom.SetTransform({10.0f, 1000.0f}, 0.0f);
        bottom.Update();

        // Add circles
        circle.SetTransform({200.0f, 200.0f}, 0.0f);
        box.SetTransform({300.0f, 200.0f}, 0.0f);

        // Set gravity
        velocityRender = world.RecordVelocity({gravity, waterForce});

        auto blendMode = vk::PipelineColorBlendAttachmentState()
                .setBlendEnable(true)
                .setAlphaBlendOp(vk::BlendOp::eAdd)
                .setColorBlendOp(vk::BlendOp::eAdd)
                .setSrcColorBlendFactor(vk::BlendFactor::eSrcAlpha)
                .setSrcAlphaBlendFactor(vk::BlendFactor::eOne)
                .setDstColorBlendFactor(vk::BlendFactor::eOneMinusSrcAlpha)
                .setDstAlphaBlendFactor(vk::BlendFactor::eZero);

        windowRender = renderTarget.Record({liquidPhi, solidPhi}, blendMode);
    }

    void Step() override
    {
        circle.Update();
        box.Update();

        sourceRender.Submit();
        world.SubmitVelocity(velocityRender);
        world.Solve();

        const int velocityStep = 8;
        const int positionStep = 3;
        rWorld.Step(delta, velocityStep, positionStep);

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
