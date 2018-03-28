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
extern glm::vec4 red;

class SmokeExample : public Runner
{
public:
    SmokeExample(const Vortex2D::Renderer::Device& device,
                 const Vortex2D::Fluid::Dimensions& dimensions,
                 float dt)
        : source1(device, glm::vec2(30.0f))
        , source2(device, glm::vec2(30.0f))
        , force1(device, glm::vec2(30.0f))
        , force2(device, glm::vec2(30.0f))
        , density(device, dimensions.Size, vk::Format::eR8G8B8A8Unorm)
        , world(device, dimensions, dt)
        , solidPhi(world.SolidDistanceField())
    {
        solidPhi.Scale = density.Scale = (glm::vec2)dimensions.Scale;
        world.FieldBind(density);

        source1.Position = force1.Position = {250.0f, 100.0f};
        source2.Position = force2.Position = {750.0f, 900.0f};

        source1.Colour = source2.Colour = red;

        force1.Colour = {0.0f, 0.05f, 0.0f, 0.0f};
        force2.Colour = {0.0f, -0.05f, 0.0f, 0.0f};

        solidPhi.Colour = green;
    }

    void Init(const Vortex2D::Renderer::Device& device,
              Vortex2D::Renderer::RenderTarget& renderTarget) override
    {
        // Draw liquid boundaries
        Vortex2D::Renderer::Rectangle area(device, glm::ivec2(1024) - glm::ivec2(24));
        area.Colour = glm::vec4(-1);
        area.Position = glm::vec2(12.0f);

        Vortex2D::Renderer::Clear clearLiquid({1.0f, 0.0f, 0.0f, 0.0f});

        world.RecordLiquidPhi({clearLiquid, area}).Submit();

        // Draw solid boundaries
        Vortex2D::Fluid::Circle obstacle1(device, 50.0f);
        Vortex2D::Fluid::Circle obstacle2(device, 50.0f);

        obstacle1.Position = {250.0f, 400.0f};
        obstacle2.Position = {750.0f, 600.0f};

        world.RecordStaticSolidPhi({Vortex2D::Fluid::BoundariesClear, obstacle1, obstacle2})
             .Submit();

        // Draw sources and forces

        velocityRender = world.RecordVelocity({force1, force2});
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

        world.Solve();

        windowRender.Submit();
    }

private:
    Vortex2D::Renderer::Ellipse source1, source2;
    Vortex2D::Renderer::Ellipse force1, force2;
    Vortex2D::Fluid::Density density;
    Vortex2D::Fluid::SmokeWorld world;
    Vortex2D::Fluid::DistanceField solidPhi;
    Vortex2D::Renderer::RenderCommand velocityRender, densityRender, windowRender;
};
