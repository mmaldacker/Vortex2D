//
//  ObstacleSmoke.h
//  Vortex2D
//

#include <Vortex2D/Vortex2D.h>

#include "Rigidbody.h"
#include "Runner.h"

#include <Box2D/Box2D.h>

#include <glm/trigonometric.hpp>

#include <functional>
#include <vector>
#include <memory>

extern glm::vec4 green;
extern glm::vec4 yellow;
extern glm::vec4 blue;

class ObstacleSmokeExample : public Runner
{
public:
    ObstacleSmokeExample(const Vortex2D::Renderer::Device& device,
                         const Vortex2D::Fluid::Dimensions& dimensions,
                         float dt)
        : delta(dt)
        , density(device, dimensions.Size, vk::Format::eR8G8B8A8Unorm)
        , world(device, dimensions, dt)
        , solidPhi(world.SolidDistanceField())
        , velocityClear({0.0f, 0.0f, 0.0f, 0.0f})
        , rWorld({0.0f, 10.0f})
        , body1(device, dimensions, rWorld, b2_dynamicBody, world, Vortex2D::Fluid::RigidBody::Type::eStatic, {100.0f, 50.0f})
        , body2(device, dimensions, rWorld, b2_dynamicBody, world, Vortex2D::Fluid::RigidBody::Type::eStatic, {50.0f, 50.0f})
        , bottom(device, dimensions, rWorld, b2_staticBody, world, Vortex2D::Fluid::RigidBody::Type::eStatic, {500.0f, 20.0f})
    {
        solidPhi.Scale = density.Scale = glm::vec2(dimensions.Scale);
        world.FieldBind(density);
        solidPhi.Colour = green;
    }

    void Init(const Vortex2D::Renderer::Device& device,
              Vortex2D::Renderer::RenderTarget& renderTarget) override
    {
        // Draw density
        Vortex2D::Renderer::Rectangle source(device, {800.0f, 400.0f});
        source.Position = {100.0f, 500.0f};
        source.Colour = yellow;

        density.Record({source}).Submit();

        // Draw liquid boundaries
        Vortex2D::Renderer::Clear clear({1.0f, 0.0f, 0.0f, 0.0f});

        Vortex2D::Renderer::Rectangle liquidArea(device, {1000.0f, 1000.0f});
        liquidArea.Colour = {-1.0f, 0.0f, 0.0f, 0.0f};
        liquidArea.Position = {12.0f, 12.0};

        world.RecordLiquidPhi({clear, liquidArea}).Submit();

        // Draw solid boundaries

        // First body
        body1.SetTransform({200.0f, 200.0f}, 45.0f);
        body1.Body().ApplyAngularImpulse(50.0f, true);
        body1.Body().ApplyForceToCenter({1000.0f, 0.0f}, true);

        // Second body
        body2.SetTransform({800.0f, 300.0f}, 0.0f);
        body2.Body().ApplyAngularImpulse(-10.0f, true);
        body2.Body().ApplyForceToCenter({-1000.0f, 0.0f}, true);

        // Bottom
        bottom.SetTransform({512.0f, 1000.5f}, 0.0f);
        bottom.Update();

        // wait for drawing to finish
        device.Handle().waitIdle();

        auto blendMode = vk::PipelineColorBlendAttachmentState()
                .setBlendEnable(true)
                .setAlphaBlendOp(vk::BlendOp::eAdd)
                .setColorBlendOp(vk::BlendOp::eAdd)
                .setSrcColorBlendFactor(vk::BlendFactor::eSrcAlpha)
                .setSrcAlphaBlendFactor(vk::BlendFactor::eOne)
                .setDstColorBlendFactor(vk::BlendFactor::eOneMinusSrcAlpha)
                .setDstAlphaBlendFactor(vk::BlendFactor::eZero);


        windowRender = renderTarget.Record({density, solidPhi}, blendMode);
    }

    void Step() override
    {
        body1.Update();
        body2.Update();

        world.Solve();

        const int velocityStep = 8;
        const int positionStep = 3;
        rWorld.Step(delta, velocityStep, positionStep);

        windowRender.Submit();
    }

private:
    float delta;
    Vortex2D::Fluid::Density density;
    Vortex2D::Fluid::SmokeWorld world;
    Vortex2D::Fluid::DistanceField solidPhi;

    Vortex2D::Renderer::Clear velocityClear;
    Vortex2D::Renderer::RenderCommand windowRender;

    b2World rWorld;

    BoxRigidbody body1;
    BoxRigidbody body2;
    BoxRigidbody bottom;
};
