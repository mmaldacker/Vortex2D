//
//  Water.h
//  Vortex2D
//

#include <Vortex2D/Vortex2D.h>

#include "Runner.h"

#include <functional>
#include <vector>
#include <memory>

extern glm::vec4 green;
extern glm::vec4 gray;
extern glm::vec4 blue;

class WaterExample : public Runner
{
public:
    WaterExample(const Vortex2D::Renderer::Device& device,
                 const Vortex2D::Fluid::Dimensions& dimensions,
                 float dt)
        : gravity(device, {1024.0f, 1024.0f}, {0.0f, 0.01f, 0.0f, 0.0f})
        , world(device, dimensions, dt)
        , solidPhi(device, world.StaticSolidPhi(), green, dimensions.Scale)
        , liquidPhi(device, world.LiquidPhi(), blue, dimensions.Scale)
    {
        liquidPhi.Scale = solidPhi.Scale = glm::vec2(dimensions.Scale);
    }

    void Init(const Vortex2D::Renderer::Device& device,
              Vortex2D::Renderer::RenderTarget& renderTarget) override
    {
        // Add particles
        Vortex2D::Renderer::IntRectangle fluid(device, {600.0f, 200.0f}, glm::ivec4(4));
        fluid.Position = {200.0f, 100.0f};

        world.Count().Record({fluid}).Submit();

        // Draw solid boundaries
        Vortex2D::Fluid::Rectangle obstacle1(device, {200.0f, 100.0f});
        Vortex2D::Fluid::Rectangle obstacle2(device, {200.0f, 100.0f});
        Vortex2D::Fluid::Rectangle area(device, {1000.0f, 1000.0f}, true);

        area.Position = glm::vec2(12.0f);

        obstacle1.Position = {300.0f, 600.0f};
        obstacle1.Rotation = 45.0f;

        obstacle2.Position = {700.0f, 600.0f};
        obstacle2.Rotation = 30.0f;

        world.StaticSolidPhi().Record({area, obstacle1, obstacle2}).Submit();

        // wait for drawing to finish
        device.Handle().waitIdle();

        // Set gravity
        velocityRender = world.Velocity().Record({gravity});

        auto blendMode = vk::PipelineColorBlendAttachmentState()
                .setBlendEnable(true)
                .setAlphaBlendOp(vk::BlendOp::eAdd)
                .setColorBlendOp(vk::BlendOp::eAdd)
                .setSrcColorBlendFactor(vk::BlendFactor::eSrcAlpha)
                .setSrcAlphaBlendFactor(vk::BlendFactor::eOne)
                .setDstColorBlendFactor(vk::BlendFactor::eOneMinusSrcAlpha)
                .setDstAlphaBlendFactor(vk::BlendFactor::eZero);

        windowRender = renderTarget.Record({solidPhi, liquidPhi}, blendMode);
    }

    void Step() override
    {
        velocityRender.Submit();
        world.Solve();

        windowRender.Submit();
    }

private:
    Vortex2D::Renderer::Rectangle gravity;
    Vortex2D::Fluid::WaterWorld world;
    Vortex2D::Fluid::DistanceField solidPhi, liquidPhi;
    Vortex2D::Renderer::RenderCommand velocityRender, windowRender;
};
