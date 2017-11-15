//
//  ObstacleSmoke.cpp
//  Vortex2D
//

#include <Vortex2D/Vortex2D.h>
#include <Vortex2D/Renderer/Drawable.h>
#include <Vortex2D/Engine/World.h>
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
        , density(device, dimensions.Size.x, dimensions.Size.y, vk::Format::eB8G8R8A8Unorm)
        , densitySprite(device, density)
        , world(device, dimensions, dt)
        , solidPhi(device, world.SolidPhi(), green, dimensions.Scale)
        , velocityClear(dimensions.Size.x, dimensions.Size.y, {0.0f, 0.0f, 0.0f, 0.0f})
        , rWorld({0.0f, 10.0f})
        , body1(device, rWorld, dimensions, world.Valid(), b2_dynamicBody, {100.0f, 50.0f})
        , body2(device, rWorld, dimensions, world.Valid(), b2_dynamicBody, {50.0f, 50.0f})
        , bottom(device, rWorld, dimensions, world.Valid(), b2_staticBody, {500.0f, 20.0f})
    {
        solidPhi.Scale = densitySprite.Scale = glm::vec2(dimensions.Scale);

        world.InitField(density);

        world.ObstacleVelocity().Record({velocityClear, body1.VelocityObject(), body2.VelocityObject()});

        // Draw density
        Vortex2D::Renderer::Rectangle source(device, {800.0f, 400.0f}, gray);
        source.Position = {100.0f, 500.0f};

        density.View = dimensions.InvScale;
        density.Record({source});
        density.Submit();
        device.Handle().waitIdle();

        // Draw liquid boundaries
        Vortex2D::Renderer::Clear clear(dimensions.Size.x, dimensions.Size.y, {1.0f, 0.0f, 0.0f, 0.0f});
        Vortex2D::Renderer::Rectangle liquidArea(device, {1000.0f, 1000.0f}, {-1.0f, 0.0f, 0.0f, 0.0f});

        liquidArea.Position = {12.0f, 12.0};

        world.LiquidPhi().View = dimensions.InvScale;
        world.LiquidPhi().Record({clear, liquidArea});
        world.LiquidPhi().Submit();
        device.Handle().waitIdle();

        // Draw solid boundaries

        // First body
        body1.SetTransform({200.0f, 200.0f}, 45.0f);
        body1.Body().ApplyAngularImpulse(50.0f, true);
        body1.Body().ApplyForceToCenter({1000.0f, 0.0f}, true);

        // Second body
        body2.SetTransform({600.0f, 300.0f}, 0.0f);
        body2.Body().ApplyAngularImpulse(-10.0f, true);
        body2.Body().ApplyForceToCenter({-1000.0f, 0.0f}, true);

        // Bottom
        bottom.SetTransform({512.5f, 1000.5f}, 0.0f);

        // Borders
        b2BodyDef border;
        b2EdgeShape line;

        line.Set({12.0f / box2dScale, 12.0f / box2dScale}, {12.0f / box2dScale, 1000.0f / box2dScale});
        auto* left = rWorld.CreateBody(&border);
        left->CreateFixture(&line, 0.0f);

        line.Set({1000.0f / box2dScale, 12.0f / box2dScale}, {1000.0f / box2dScale, 1000.0f / box2dScale});
        auto* right = rWorld.CreateBody(&border);
        right->CreateFixture(&line, 0.0f);

        world.SolidPhi().View = dimensions.InvScale;
        world.SolidPhi().DrawSignedObject({bottom.SignedObject(), body1.SignedObject(), body2.SignedObject()});
    }

    void Initialize(const Vortex2D::Renderer::RenderState& renderState) override
    {
        densitySprite.Initialize(renderState);
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

        densitySprite.Update(projection, view);
        solidPhi.Update(projection, view);

        world.SolidPhi().SubmitSignedBoject();
        world.SolidPhi().Reinitialise();
        world.ObstacleVelocity().Submit();

        world.SolveStatic();

        const int velocityStep = 8;
        const int positionStep = 3;
        rWorld.Step(delta, velocityStep, positionStep);
    }

    void Draw(vk::CommandBuffer commandBuffer, const Vortex2D::Renderer::RenderState& renderState) override
    {
        densitySprite.Draw(commandBuffer, renderState);
        solidPhi.Draw(commandBuffer, renderState);
    }

private:
    float delta;
    Vortex2D::Fluid::Dimensions dimensions;
    Vortex2D::Renderer::RenderTexture density;
    Vortex2D::Renderer::Sprite densitySprite;
    Vortex2D::Fluid::World world;
    Vortex2D::Fluid::DistanceField solidPhi;

    Vortex2D::Renderer::Clear velocityClear;

    b2World rWorld;

    BoxRigidbody body1;
    BoxRigidbody body2;
    BoxRigidbody bottom;
};
