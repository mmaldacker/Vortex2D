//
//  Smoke.h
//  Vortex
//

#pragma once

#include <Vortex/Vortex.h>

#include "Runner.h"

#include <functional>
#include <memory>
#include <vector>

extern glm::vec4 green;
extern glm::vec4 gray;

class SmokeExample : public Runner
{
public:
  SmokeExample(Vortex::Renderer::Device& device, const glm::ivec2& size, float dt)
      : density(std::make_shared<Vortex::Fluid::Density>(device,
                                                         size,
                                                         Vortex::Renderer::Format::R8G8B8A8Unorm))
      , world(device, size, dt, Vortex::Fluid::Velocity::InterpolationMode::Linear)
  {
    world.FieldBind(*density);
  }

  void Init(Vortex::Renderer::Device& device, Vortex::Renderer::RenderTarget& renderTarget) override
  {
    auto source1 = std::make_shared<Vortex::Renderer::Rectangle>(device, glm::vec2(20.0f));
    auto source2 = std::make_shared<Vortex::Renderer::Rectangle>(device, glm::vec2(20.0f));
    auto force1 = std::make_shared<Vortex::Renderer::Rectangle>(device, glm::vec2(20.0f));
    auto force2 = std::make_shared<Vortex::Renderer::Rectangle>(device, glm::vec2(20.0f));

    source1->Position = force1->Position = {75.0f, 25.0f};
    source2->Position = force2->Position = {175.0f, 225.0f};

    source1->Anchor = source2->Anchor = glm::vec2(10.0);
    force1->Anchor = force2->Anchor = glm::vec2(10.0);

    source1->Colour = source2->Colour = gray;

    force1->Colour = {0.0f, 30.0f, 0.0f, 0.0f};
    force2->Colour = {0.0f, -30.0f, 0.0f, 0.0f};

    auto solidPhi = world.MakeSolidDistanceField();
    solidPhi->Colour = green;

    // Draw liquid boundaries
    auto area =
        std::make_shared<Vortex::Renderer::Rectangle>(device, glm::ivec2(256) - glm::ivec2(4));
    area->Colour = glm::vec4(-1);
    area->Position = glm::vec2(2.0f);

    auto clearLiquid = std::make_shared<Vortex::Renderer::Clear>(glm::vec4{1.0f, 0.0f, 0.0f, 0.0f});

    world.RecordLiquidPhi({clearLiquid, area}).Submit().Wait();

    // Draw solid boundaries
    auto obstacle1 = std::make_shared<Vortex::Fluid::Circle>(device, 15.0f);
    auto obstacle2 = std::make_shared<Vortex::Fluid::Circle>(device, 15.0f);

    obstacle1->Position = {75.0f, 100.0f};
    obstacle2->Position = {175.0f, 125.0f};

    world.RecordStaticSolidPhi({Vortex::Fluid::BoundariesClear, obstacle1, obstacle2})
        .Submit()
        .Wait();

    // Draw sources and forces
    velocityRender = world.RecordVelocity({force1, force2}, Vortex::Fluid::VelocityOp::Set);
    densityRender = density->Record({source1, source2});

    Vortex::Renderer::ColorBlendState blendState(Vortex::Renderer::BlendFactor::SrcAlpha,
                                                 Vortex::Renderer::BlendFactor::OneMinusSrcAlpha,
                                                 Vortex::Renderer::BlendOp::Add,
                                                 Vortex::Renderer::BlendFactor::One,
                                                 Vortex::Renderer::BlendFactor::Zero,
                                                 Vortex::Renderer::BlendOp::Add);

    windowRender = renderTarget.Record({density, solidPhi}, blendState);
  }

  void Step() override
  {
    velocityRender.Submit();
    densityRender.Submit();

    auto params = Vortex::Fluid::FixedParams(12);
    world.Step(params);

    windowRender.Submit();
  }

private:
  std::shared_ptr<Vortex::Fluid::Density> density;
  Vortex::Fluid::SmokeWorld world;
  Vortex::Renderer::RenderCommand velocityRender, densityRender, windowRender;
};
