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
extern glm::vec4 gray;

class ObstacleSmokeExample : public Runner
{
public:
    ObstacleSmokeExample(const Vortex2D::Renderer::Device& device,
                         const glm::ivec2& size,
                         float dt)
        : density(device, size, vk::Format::eR8G8B8A8Unorm)
        , world(device, size, dt)
        , solidPhi(world.SolidDistanceField())
        , velocityClear({0.0f, 0.0f, 0.0f, 0.0f})
        , rWorld({0.0f, 100.0f})
        , solver(rWorld)
        , body1(device, size, rWorld, b2_dynamicBody, Vortex2D::Fluid::RigidBody::Type::eStatic, {25.0f, 12.0f})
        , body2(device, size, rWorld, b2_dynamicBody, Vortex2D::Fluid::RigidBody::Type::eStatic, {12.0f, 12.0f})
        , bottom(device, size, rWorld, b2_staticBody, Vortex2D::Fluid::RigidBody::Type::eStatic, {125.0f, 5.0f})
    {
        world.FieldBind(density);
        world.AttachRigidBodySolver(solver);
        world.AddRigidbody(body1.mRigidbody);
        world.AddRigidbody(body2.mRigidbody);
        world.AddRigidbody(bottom.mRigidbody);

        solidPhi.Colour = green;
    }

    void Init(const Vortex2D::Renderer::Device& device,
              Vortex2D::Renderer::RenderTarget& renderTarget) override
    {
        // Draw density
        Vortex2D::Renderer::Rectangle source(device, {200, 100.0f});
        source.Position = {25.0f, 125.0f};
        source.Colour = gray;

        density.Record({source}).Submit().Wait();

        // Draw liquid boundaries
        Vortex2D::Renderer::Clear clear({1.0f, 0.0f, 0.0f, 0.0f});

        Vortex2D::Renderer::Rectangle liquidArea(device, {250.0f, 250.0f});
        liquidArea.Colour = {-1.0f, 0.0f, 0.0f, 0.0f};
        liquidArea.Position = {3.0f, 3.0};

        world.RecordLiquidPhi({clear, liquidArea}).Submit().Wait();

        // Draw solid boundaries

        // First body
        body1.mRigidbody.SetTransform({50.0f, 50.0f}, 45.0f);
        body1.mRigidbody.mBody->ApplyAngularImpulse(5e5f, true);
        body1.mRigidbody.mBody->ApplyForceToCenter({1e5f, 0.0f}, true);

        // Second body
        body2.mRigidbody.SetTransform({200.0f, 80.0f}, 0.0f);
        body2.mRigidbody.mBody->ApplyAngularImpulse(-1e5f, true);
        body2.mRigidbody.mBody->ApplyForceToCenter({-1e5f, 0.0f}, true);

        // Bottom
        bottom.mRigidbody.SetTransform({128.0f, 256.5f}, 0.0f);

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
        auto params = Vortex2D::Fluid::FixedParams(12);
        world.Step(params);
        windowRender.Submit();
    }

private:
    Vortex2D::Fluid::Density density;
    Vortex2D::Fluid::SmokeWorld world;
    Vortex2D::Fluid::DistanceField solidPhi;

    Vortex2D::Renderer::Clear velocityClear;
    Vortex2D::Renderer::RenderCommand windowRender;

    b2World rWorld;

    Box2DSolver solver;
    RectangleRigidbody body1;
    RectangleRigidbody body2;
    RectangleRigidbody bottom;
};
