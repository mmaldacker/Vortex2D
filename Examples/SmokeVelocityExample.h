//
//  SmokeVelocity.cpp
//  Vortex2D
//

#include <Vortex2D/Vortex2D.h>
#include <Vortex2D/Renderer/Drawable.h>
#include <Vortex2D/Engine/World.h>
#include <Vortex2D/Renderer/RenderTexture.h>
#include <Vortex2D/Renderer/Sprite.h>
#include <Vortex2D/Engine/Boundaries.h>

#include <Box2D/Box2D.h>

#include "Rigidbody.h"

#include <functional>
#include <vector>
#include <memory>

extern glm::vec4 green;
extern glm::vec4 gray;

class SmokeVelocityExample : public Vortex2D::Renderer::Drawable
{
public:
    SmokeVelocityExample(const Vortex2D::Renderer::Device& device,
                 const Vortex2D::Fluid::Dimensions& dimensions,
                 float dt)
        : delta(dt)
        , dimensions(dimensions)
        , source(device, glm::vec2(20.0f), gray)
        , force(device, glm::vec2(20.0f), {0.5f, 0.5f, 0.0f, 0.0f})
        , density(device, dimensions.Size.x, dimensions.Size.y, vk::Format::eB8G8R8A8Unorm)
        , densitySprite(device, density)
        , world(device, dimensions, dt)
        , solidPhi(device, world.SolidPhi(), green, dimensions.Scale)
        , rWorld({0.0f, 0.0f})
        , body(device, rWorld, dimensions, world.Valid(), b2_dynamicBody, {200.0f, 50.0f})
    {
        solidPhi.Scale = densitySprite.Scale = (glm::vec2)dimensions.Scale;

        source.Position = force.Position = {100.0f, 100.0f};

        // Draw liquid boundaries
        Vortex2D::Renderer::Rectangle area(device, dimensions.Size - glm::ivec2(2.0f), glm::vec4(-1.0f));
        Vortex2D::Renderer::Clear clearLiquid(dimensions.Size.x, dimensions.Size.y, {1.0f, 0.0f, 0.0f, 0.0f});

        area.Position = glm::vec2(1.0f);

        world.LiquidPhi().Record({clearLiquid, area});
        world.LiquidPhi().Submit();
        device.Handle().waitIdle();

        // Draw sources and forces
        world.InitField(density);

        world.Velocity().View = dimensions.InvScale;
        world.Velocity().Record({force});

        density.View = dimensions.InvScale;
        density.Record({source});

        // Draw rigid body
        body.SetTransform({300.0f, 500.0f}, -45.0f);
        world.SolidPhi().View = dimensions.InvScale;
        world.SolidPhi().DrawSignedObject({body.SignedObject()});
    }

    void Initialize(const Vortex2D::Renderer::RenderState& renderState) override
    {
        densitySprite.Initialize(renderState);
        solidPhi.Initialize(renderState);
    }

    void Update(const glm::mat4& projection, const glm::mat4& view) override
    {
        body.UpdatePosition();
        body.UpdateVelocities();
        body.Update(world.SolidPhi().Orth, dimensions.InvScale);

        densitySprite.Update(projection, view);
        solidPhi.Update(projection, view);

        world.SolidPhi().SubmitSignedBoject();
        world.Velocity().Submit();
        density.Submit();

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
    Vortex2D::Renderer::Ellipse source;
    Vortex2D::Renderer::Ellipse force;
    Vortex2D::Renderer::RenderTexture density;
    Vortex2D::Renderer::Sprite densitySprite;
    Vortex2D::Fluid::World world;
    Vortex2D::Fluid::DistanceField solidPhi;

    b2World rWorld;

    BoxRigidbody body;
};
