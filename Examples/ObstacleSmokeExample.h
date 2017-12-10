//
//  ObstacleSmoke.cpp
//  Vortex2D
//

#include <Vortex2D/Vortex2D.h>
#include <Vortex2D/Renderer/Drawable.h>
#include <Vortex2D/Engine/World.h>
#include <Vortex2D/Engine/Density.h>
#include <Vortex2D/Renderer/RenderTexture.h>
#include <Vortex2D/Renderer/Sprite.h>
#include <Vortex2D/Engine/Boundaries.h>

#include "Rigidbody.h"

#include <Box2D/Box2D.h>

#include <glm/trigonometric.hpp>

#include <functional>
#include <vector>
#include <memory>

extern glm::vec4 green;
extern glm::vec4 gray;
extern glm::vec4 blue;

class ObstacleSmokeExample : public Vortex2D::Renderer::Drawable
{
public:
    ObstacleSmokeExample(const Vortex2D::Renderer::Device& device,
                         const Vortex2D::Fluid::Dimensions& dimensions,
                         float dt)
        : delta(dt)
        , dimensions(dimensions)
        , density(device, dimensions.Size, vk::Format::eB8G8R8A8Unorm)
        , world(device, dimensions, dt)
        , solidPhi(device, world.SolidPhi(), green, dimensions.Scale)
        , velocityClear({0.0f, 0.0f, 0.0f, 0.0f})
        , rWorld({0.0f, 10.0f})
        , body1(device, rWorld, dimensions, world.Valid(), b2_dynamicBody, {100.0f, 50.0f})
        , body2(device, rWorld, dimensions, world.Valid(), b2_dynamicBody, {50.0f, 50.0f})
        , bottom(device, rWorld, dimensions, world.Valid(), b2_staticBody, {500.0f, 20.0f})
    {
        solidPhi.Scale = density.Scale = glm::vec2(dimensions.Scale);

        world.InitField(density);

        velocityRender = world.ObstacleVelocity().Record({velocityClear, body1.VelocityObject(), body2.VelocityObject()});

        // Draw density
        Vortex2D::Renderer::Rectangle source(device, {800.0f, 400.0f}, gray);
        source.Position = {100.0f, 500.0f};

        density.View = dimensions.InvScale;
        density.Record({source}).Submit();

        // Draw liquid boundaries
        Vortex2D::Renderer::Clear clear({1.0f, 0.0f, 0.0f, 0.0f});
        Vortex2D::Renderer::Rectangle liquidArea(device, {1000.0f, 1000.0f}, {-1.0f, 0.0f, 0.0f, 0.0f});

        liquidArea.Position = {12.0f, 12.0};

        world.LiquidPhi().View = dimensions.InvScale;
        world.LiquidPhi().Record({clear, liquidArea}).Submit();

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
        bottom.SetTransform({512.5f, 1000.5f}, 0.0f);

        world.SolidPhi().View = dimensions.InvScale;
        world.SolidPhi().DrawSignedObject({bottom.SignedObject(), body1.SignedObject(), body2.SignedObject()});

        // wait for drawing to finish
        device.Handle().waitIdle();
    }

    void Initialize(const Vortex2D::Renderer::RenderState& renderState) override
    {
        density.Initialize(renderState);
        solidPhi.Initialize(renderState);
    }

    void Update(const glm::mat4& projection, const glm::mat4& view) override
    {
        body1.UpdatePosition();
        body1.UpdateVelocities();
        body1.Update(world.SolidPhi().Orth, dimensions.InvScale);

        body2.UpdatePosition();
        body2.UpdateVelocities();
        body2.Update(world.SolidPhi().Orth, dimensions.InvScale);

        density.Update(projection, view);
        solidPhi.Update(projection, view);

        world.SolidPhi().SubmitSignedBoject();
        world.SolidPhi().Reinitialise();
        velocityRender.Submit();

        world.SolveStatic();

        const int velocityStep = 8;
        const int positionStep = 3;
        rWorld.Step(delta, velocityStep, positionStep);
    }

    void Draw(vk::CommandBuffer commandBuffer, const Vortex2D::Renderer::RenderState& renderState) override
    {
        density.Draw(commandBuffer, renderState);
        solidPhi.Draw(commandBuffer, renderState);
    }

private:
    float delta;
    Vortex2D::Fluid::Dimensions dimensions;
    Vortex2D::Fluid::Density density;
    Vortex2D::Fluid::World world;
    Vortex2D::Fluid::DistanceField solidPhi;

    Vortex2D::Renderer::Clear velocityClear;
    Vortex2D::Renderer::RenderCommand velocityRender;

    b2World rWorld;

    BoxRigidbody body1;
    BoxRigidbody body2;
    BoxRigidbody bottom;
};
