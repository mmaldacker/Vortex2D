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
                         const glm::ivec2& size,
                         float dt)
        : delta(dt)
        , density(device, size, vk::Format::eR8G8B8A8Unorm)
        , world(device, size, dt)
        , solidPhi(world.SolidDistanceField())
        , velocityClear({0.0f, 0.0f, 0.0f, 0.0f})
        , rWorld({0.0f, 100.0f})
        , body1(device, rWorld, b2_dynamicBody, world, Vortex2D::Fluid::RigidBody::Type::eStatic, {25.0f, 12.0f})
        , body2(device, rWorld, b2_dynamicBody, world, Vortex2D::Fluid::RigidBody::Type::eStatic, {12.0f, 12.0f})
        , bottom(device, rWorld, b2_staticBody, world, Vortex2D::Fluid::RigidBody::Type::eStatic, {125.0f, 5.0f})
    {
        world.FieldBind(density);
        solidPhi.Colour = green;
    }

    void Init(const Vortex2D::Renderer::Device& device,
              Vortex2D::Renderer::RenderTarget& renderTarget) override
    {
        // Draw density
        Vortex2D::Renderer::Rectangle source(device, {200, 100.0f});
        source.Position = {25.0f, 125.0f};
        source.Colour = yellow;

        density.Record({source}).Submit().Wait();

        // Draw liquid boundaries
        Vortex2D::Renderer::Clear clear({1.0f, 0.0f, 0.0f, 0.0f});

        Vortex2D::Renderer::Rectangle liquidArea(device, {250.0f, 250.0f});
        liquidArea.Colour = {-1.0f, 0.0f, 0.0f, 0.0f};
        liquidArea.Position = {3.0f, 3.0};

        world.RecordLiquidPhi({clear, liquidArea}).Submit().Wait();

        // Draw solid boundaries

        // First body
        body1.SetTransform({50.0f, 50.0f}, 45.0f);
        body1.Body().ApplyAngularImpulse(5e5f, true);
        body1.Body().ApplyForceToCenter({1e5f, 0.0f}, true);

        // Second body
        body2.SetTransform({200.0f, 80.0f}, 0.0f);
        body2.Body().ApplyAngularImpulse(-1e5f, true);
        body2.Body().ApplyForceToCenter({-1e5f, 0.0f}, true);

        // Bottom
        bottom.SetTransform({128.0f, 256.5f}, 0.0f);
        bottom.Update();

        Vortex2D::Renderer::ColorBlendState blendState;
        blendState.ColorBlend
                .setBlendEnable(true)
                .setAlphaBlendOp(vk::BlendOp::eAdd)
                .setColorBlendOp(vk::BlendOp::eAdd)
                .setSrcColorBlendFactor(vk::BlendFactor::eSrcAlpha)
                .setSrcAlphaBlendFactor(vk::BlendFactor::eOne)
                .setDstColorBlendFactor(vk::BlendFactor::eOneMinusSrcAlpha)
                .setDstAlphaBlendFactor(vk::BlendFactor::eZero);


        windowRender = renderTarget.Record({density, solidPhi}, blendState);
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
