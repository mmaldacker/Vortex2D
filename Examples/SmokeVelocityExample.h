//
//  SmokeVelocity.cpp
//  Vortex2D
//

#include <Vortex2D/Vortex2D.h>

#include <Box2D/Box2D.h>

#include "Rigidbody.h"
#include "Runner.h"

#include <functional>
#include <vector>
#include <memory>

extern glm::vec4 green;
extern glm::vec4 gray;

class SmokeVelocityExample : public Runner
{
public:
    SmokeVelocityExample(const Vortex2D::Renderer::Device& device,
                 const Vortex2D::Fluid::Dimensions& dimensions,
                 float dt)
        : delta(dt)
        , dimensions(dimensions)
        , source(device, glm::vec2(20.0f), gray)
        , force(device, glm::vec2(20.0f), {0.5f, 0.5f, 0.0f, 0.0f})
        , density(device, dimensions.Size, vk::Format::eB8G8R8A8Unorm)
        , world(device, dimensions, dt)
        , solidPhi(device, world.DynamicSolidPhi(), green, dimensions.Scale)
        , clearObstacles({1000.0f, 0.0f, 0.0f, 0.0f})
        , rWorld({0.0f, 0.0f})
        , body(device, rWorld, dimensions, world, b2_dynamicBody, {200.0f, 50.0f})
    {
        solidPhi.Scale = density.Scale = (glm::vec2)dimensions.Scale;

        source.Position = force.Position = {100.0f, 100.0f};
    }

    void Init(const Vortex2D::Renderer::Device& device,
              Vortex2D::Renderer::RenderTarget& renderTarget) override
    {
        // Draw liquid boundaries
        Vortex2D::Renderer::Rectangle area(device, dimensions.Size - glm::ivec2(2.0f), glm::vec4(-1.0f));
        Vortex2D::Renderer::Clear clearLiquid({1.0f, 0.0f, 0.0f, 0.0f});

        area.Position = glm::vec2(1.0f);

        world.LiquidPhi().Record({clearLiquid, area}).Submit();

        // Draw sources and forces
        world.InitField(density);

        world.Velocity().View = dimensions.InvScale;
        velocityRender = world.Velocity().Record({force});

        density.View = dimensions.InvScale;
        densityRender = density.Record({source});

        // Draw rigid body
        body.SetTransform({300.0f, 500.0f}, -45.0f);
        obstaclesRender = world.StaticSolidPhi().Record({clearObstacles, body.SignedObject()},
                                                        Vortex2D::Fluid::UnionBlend);

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
        body.UpdatePosition();
        body.UpdateVelocities();

        obstaclesRender.Submit();
        velocityRender.Submit();
        densityRender.Submit();

        world.SolveStatic();

        const int velocityStep = 8;
        const int positionStep = 3;
        rWorld.Step(delta, velocityStep, positionStep);

        windowRender.Submit();
    }

private:
    float delta;
    Vortex2D::Fluid::Dimensions dimensions;
    Vortex2D::Renderer::Ellipse source;
    Vortex2D::Renderer::Ellipse force;
    Vortex2D::Fluid::Density density;
    Vortex2D::Fluid::World world;
    Vortex2D::Fluid::DistanceField solidPhi;
    Vortex2D::Renderer::Clear clearObstacles;
    Vortex2D::Renderer::RenderCommand velocityRender, densityRender, obstaclesRender, windowRender;

    b2World rWorld;

    BoxRigidbody body;
};
