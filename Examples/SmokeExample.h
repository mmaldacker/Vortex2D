//
//  Smoke.h
//  Vortex2D
//

#include <Vortex2D/Vortex2D.h>

#include "Runner.h"

#include <functional>
#include <vector>
#include <memory>

extern glm::vec4 green;
extern glm::vec4 gray;

class SmokeExample : public Runner
{
public:
    SmokeExample(const Vortex2D::Renderer::Device& device,
                 const Vortex2D::Fluid::Dimensions& dimensions,
                 float dt)
        : dimensions(dimensions)
        , source1(device, glm::vec2(20.0f), gray)
        , source2(device, glm::vec2(20.0f), gray)
        , force1(device, glm::vec2(20.0f), {0.0f, 0.5f, 0.0f, 0.0f})
        , force2(device, glm::vec2(20.0f), {0.0f, -0.5f, 0.0f, 0.0f})
        , density(device, dimensions.Size, vk::Format::eB8G8R8A8Unorm)
        , world(device, dimensions, dt)
        , solidPhi(device, world.SolidPhi(), green, dimensions.Scale)
    {
        solidPhi.Scale = density.Scale = (glm::vec2)dimensions.Scale;

        source1.Position = force1.Position = {250.0f, 100.0f};
        source2.Position = force2.Position = {750.0f, 900.0f};
    }

    void Init(const Vortex2D::Renderer::Device& device,
              Vortex2D::Renderer::RenderTarget& renderTarget) override
    {
        // Draw liquid boundaries
        Vortex2D::Renderer::Rectangle area(device, dimensions.Size - glm::ivec2(2.0f), glm::vec4(-1.0f));
        Vortex2D::Renderer::Clear clearLiquid({1.0f, 0.0f, 0.0f, 0.0f});

        area.Position = glm::vec2(1.0f);

        world.LiquidPhi().Record({clearLiquid, area}).Submit();

        // Draw solid boundaries
        Vortex2D::Renderer::Clear clearObstacles({1000.0, 0.0f, 0.0f, 0.0f});
        Vortex2D::Fluid::Circle obstacle1(device, 50.0f);
        Vortex2D::Fluid::Circle obstacle2(device, 50.0f);

        obstacle1.Position = {250.0f, 400.0f};
        obstacle2.Position = {750.0f, 600.0f};

        world.SolidPhi().View = dimensions.InvScale;
        world.SolidPhi().Record({clearObstacles, obstacle1, obstacle2}, Vortex2D::Fluid::UnionBlend).Submit();

        // Draw sources and forces
        world.InitField(density);

        world.Velocity().View = dimensions.InvScale;
        density.View = dimensions.InvScale;

        velocityRender = world.Velocity().Record({force1, force2});
        densityRender = density.Record({source1, source2});

        // wait for all drawing to finish
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
        velocityRender.Submit();
        densityRender.Submit();

        world.SolveStatic();

        windowRender.Submit();
    }

private:
    const Vortex2D::Fluid::Dimensions& dimensions;
    Vortex2D::Renderer::Ellipse source1, source2;
    Vortex2D::Renderer::Ellipse force1, force2;
    Vortex2D::Fluid::Density density;
    Vortex2D::Fluid::World world;
    Vortex2D::Fluid::DistanceField solidPhi;
    Vortex2D::Renderer::RenderCommand velocityRender, densityRender, windowRender;
};
