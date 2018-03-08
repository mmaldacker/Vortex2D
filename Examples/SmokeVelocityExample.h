//
//  SmokeVelocity.h
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
extern glm::vec4 yellow;

class SmokeVelocityExample : public Runner
{
public:
    SmokeVelocityExample(const Vortex2D::Renderer::Device& device,
                 const Vortex2D::Fluid::Dimensions& dimensions,
                 float dt)
        : delta(dt)
        , source1(device, glm::vec2(20.0f))
        , source2(device, glm::vec2(20.0f))
        , force1(device, glm::vec2(20.0f))
        , force2(device, glm::vec2(20.0f))
        , density(device, dimensions.Size, vk::Format::eR8G8B8A8Unorm)
        , world(device, dimensions, dt)
        , clearObstacles({1000.0f, 0.0f, 0.0f, 0.0f})
        , rWorld({0.0f, 0.0f})
        , body(device, dimensions, rWorld, b2_dynamicBody, world, Vortex2D::Fluid::RigidBody::Type::eWeak, {200.0f, 50.0f})
        , solidPhi(device, body.Phi(), dimensions.Scale)
    {
        solidPhi.Scale = density.Scale = (glm::vec2)dimensions.Scale;
        density.View = dimensions.InvScale;
        world.FieldBind(density);
        source1.Position = force1.Position = {100.0f, 100.0f};
        source2.Position = force2.Position = {500.0f, 900.0f};

        source1.Colour = source2.Colour = yellow;

        force1.Colour = {0.5f, 0.5f, 0.0f, 0.0f};
        force2.Colour = {-0.5f, -0.5f, 0.0f, 0.0f};

        solidPhi.Colour = green;
    }

    void Init(const Vortex2D::Renderer::Device& device,
              Vortex2D::Renderer::RenderTarget& renderTarget) override
    {
        // Draw liquid boundaries
        Vortex2D::Renderer::Rectangle area(device, glm::ivec2(1024) - glm::ivec2(24));
        area.Colour = glm::vec4(-1.0f);
        area.Position = glm::vec2(1.0f);

        Vortex2D::Renderer::Clear clearLiquid({1.0f, 0.0f, 0.0f, 0.0f});

        world.RecordLiquidPhi({clearLiquid, area}).Submit();

        // Draw sources and forces
        velocityRender = world.RecordVelocity({force1, force2});
        densityRender = density.Record({source1, source2});

        // Draw rigid body
        body.SetTransform({300.0f, 500.0f}, -45.0f);

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
        body.Update();

        velocityRender.Submit();
        densityRender.Submit();

        world.Solve();

        const int velocityStep = 8;
        const int positionStep = 3;
        rWorld.Step(delta, velocityStep, positionStep);

        windowRender.Submit();
    }

private:
    float delta;
    Vortex2D::Renderer::Ellipse source1, source2;
    Vortex2D::Renderer::Ellipse force1, force2;
    Vortex2D::Fluid::Density density;
    Vortex2D::Fluid::SmokeWorld world;
    Vortex2D::Renderer::Clear clearObstacles;
    Vortex2D::Renderer::RenderCommand velocityRender, densityRender, windowRender;

    b2World rWorld;

    BoxRigidbody body;
    Vortex2D::Fluid::DistanceField solidPhi;
};
