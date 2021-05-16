//
//  Water.h
//  Vortex2D
//

#include <Vortex2D/Vortex2D.h>

#include "Runner.h"

#include <functional>
#include <memory>
#include <vector>

extern glm::vec4 green;
extern glm::vec4 blue;

class WaterExample : public Runner
{
public:
  WaterExample(const Vortex::Renderer::Device& device, const glm::ivec2& size, float dt)
      : gravity(device, {256.0f, 256.0f})
      , world(device, size, dt, 2, Vortex::Fluid::Velocity::InterpolationMode::Linear)
      , solidPhi(world.SolidDistanceField())
      , liquidPhi(world.LiquidDistanceField())
  {
    gravity.Colour = {0.0f, 3.0f, 0.0f, 0.0f};

    solidPhi.Colour = green;
    liquidPhi.Colour = blue;
  }

  void Init(const Vortex::Renderer::Device& device,
            Vortex::Renderer::RenderTarget& renderTarget) override
  {
    // Add particles
    Vortex::Renderer::IntRectangle fluid(device, {150.0f, 50.0f});
    fluid.Position = {50.0f, 25.0f};
    fluid.Colour = glm::vec4(4);

    world.RecordParticleCount({fluid}).Submit().Wait();

    // Draw solid boundaries
    Vortex::Fluid::Rectangle obstacle1(device, {50.0f, 25.0f});
    Vortex::Fluid::Rectangle obstacle2(device, {50.0f, 25.0f});
    Vortex::Fluid::Rectangle area(device, {250.0f, 250.0f}, true, 5.0f);

    area.Position = glm::vec2(3.0f);

    obstacle1.Position = {75.0f, 150.0f};
    obstacle1.Rotation = 45.0f;

    obstacle2.Position = {150.0f, 150.0f};
    obstacle2.Rotation = 30.0f;

    world.RecordStaticSolidPhi({area, obstacle1, obstacle2}).Submit().Wait();

    // Set gravity
    velocityRender = world.RecordVelocity({gravity}, Vortex::Fluid::VelocityOp::Add);

    Vortex::Renderer::ColorBlendState blendState;
    blendState.ColorBlend.setBlendEnable(true)
        .setAlphaBlendOp(vk::BlendOp::eAdd)
        .setColorBlendOp(vk::BlendOp::eAdd)
        .setSrcColorBlendFactor(vk::BlendFactor::eSrcAlpha)
        .setSrcAlphaBlendFactor(vk::BlendFactor::eOne)
        .setDstColorBlendFactor(vk::BlendFactor::eOneMinusSrcAlpha)
        .setDstAlphaBlendFactor(vk::BlendFactor::eZero);

    windowRender = renderTarget.Record({liquidPhi, solidPhi}, blendState);
  }

  void Step() override
  {
    world.SubmitVelocity(velocityRender);
    auto params = Vortex::Fluid::FixedParams(12);
    world.Step(params);

    windowRender.Submit();
  }

private:
  Vortex::Renderer::Rectangle gravity;
  Vortex::Fluid::WaterWorld world;
  Vortex::Fluid::DistanceField solidPhi, liquidPhi;
  Vortex::Renderer::RenderCommand velocityRender, windowRender;
};
