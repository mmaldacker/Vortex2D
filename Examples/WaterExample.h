//
//  Water.h
//  Vortex
//

#pragma once

#include <Vortex/Vortex.h>

#include "Runner.h"

#include <functional>
#include <memory>
#include <vector>

extern glm::vec4 green;
extern glm::vec4 blue;

class WaterExample : public Runner
{
public:
  WaterExample(Vortex::Renderer::Device& device, const glm::ivec2& size, float dt)
      : world(device, size, dt, 2, Vortex::Fluid::Velocity::InterpolationMode::Linear)

  {
  }

  void Init(Vortex::Renderer::Device& device,
            Vortex::Renderer::RenderTarget& renderTarget) override
  {
    auto gravity = std::make_shared<Vortex::Renderer::Rectangle>(device, glm::vec2{256.0f, 256.0f});
    gravity->Colour = {0.0f, 3.0f, 0.0f, 0.0f};

    // Add particles
    auto fluid = std::make_shared<Vortex::Renderer::IntRectangle>(device, glm::vec2{150.0f, 50.0f});
    fluid->Position = {50.0f, 25.0f};
    fluid->Colour = glm::vec4(4);

    world.RecordParticleCount({fluid}).Submit().Wait();

    // Draw solid boundaries
    auto obstacle1 = std::make_shared<Vortex::Fluid::Rectangle>(device, glm::vec2{50.0f, 25.0f});
    auto obstacle2 = std::make_shared<Vortex::Fluid::Rectangle>(device, glm::vec2{50.0f, 25.0f});
    auto area =
        std::make_shared<Vortex::Fluid::Rectangle>(device, glm::vec2{250.0f, 250.0f}, true, 5.0f);

    area->Position = glm::vec2(3.0f);

    obstacle1->Position = {75.0f, 150.0f};
    obstacle1->Rotation = 45.0f;

    obstacle2->Position = {150.0f, 150.0f};
    obstacle2->Rotation = 30.0f;

    world.RecordStaticSolidPhi({area, obstacle1, obstacle2}).Submit().Wait();

    // Set gravity
    velocityRender = world.RecordVelocity({gravity}, Vortex::Fluid::VelocityOp::Add);

    auto solidPhi = world.MakeSolidDistanceField();
    auto liquidPhi = world.MakeLiquidDistanceField();
    solidPhi->Colour = green;
    liquidPhi->Colour = blue;

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
  Vortex::Fluid::WaterWorld world;
  Vortex::Renderer::RenderCommand velocityRender, windowRender;
};
