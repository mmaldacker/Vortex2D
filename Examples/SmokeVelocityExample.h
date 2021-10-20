//
//  SmokeVelocity.h
//  Vortex
//

#pragma once

#include <Vortex/Vortex.h>

#include <Box2D/Box2D.h>

#include "Rigidbody.h"
#include "Runner.h"

#include <functional>
#include <memory>
#include <vector>

extern glm::vec4 green;
extern glm::vec4 gray;

class SmokeVelocityExample : public Runner
{
public:
  SmokeVelocityExample(Vortex::Renderer::Device& device, const glm::ivec2& size, float dt)
      : density(std::make_shared<Vortex::Fluid::Density>(device, size, vk::Format::eR8G8B8A8Unorm))
      , world(device, size, dt, Vortex::Fluid::Velocity::InterpolationMode::Linear)
      , rWorld({0.0f, 0.0f})
      , solver(rWorld)
      , body(device,
             size,
             rWorld,
             b2_dynamicBody,
             Vortex::Fluid::RigidBody::Type::eWeak,
             {80.0f, 10.0f},
             2.0f)
  {
    world.FieldBind(*density);
    world.AttachRigidBodySolver(solver);
    world.AddRigidbody(body.mRigidbody);
  }

  void Init(Vortex::Renderer::Device& device,
            Vortex::Renderer::RenderTarget& renderTarget) override
  {
    auto source1 = std::make_shared<Vortex::Renderer::Ellipse>(device, glm::vec2(5.0f));
    auto source2 = std::make_shared<Vortex::Renderer::Ellipse>(device, glm::vec2(5.0f));
    auto force1 = std::make_shared<Vortex::Renderer::Ellipse>(device, glm::vec2(5.0f));
    auto force2 = std::make_shared<Vortex::Renderer::Ellipse>(device, glm::vec2(5.0f));

    source1->Position = force1->Position = {25.0f, 25.0f};
    source2->Position = force2->Position = {125.0f, 225.0f};

    source1->Colour = source2->Colour = gray;

    force1->Colour = {50.0f, 0.0f, 0.0f, 0.0f};
    force2->Colour = {-50.0f, -50.0f, 0.0f, 0.0f};

    auto solidPhi = std::make_shared<Vortex::Fluid::DistanceField>(device, body.mRigidbody.Phi());
    solidPhi->Colour = green;

    // Draw liquid boundaries
    auto area = std::make_shared<Vortex::Renderer::Rectangle>(device, glm::ivec2(250));
    area->Colour = glm::vec4(-1.0f);
    area->Position = glm::vec2(3.0f);

    auto clearLiquid = std::make_shared<Vortex::Renderer::Clear>(glm::vec4{1.0f, 0.0f, 0.0f, 0.0f});

    world.RecordLiquidPhi({clearLiquid, area}).Submit().Wait();

    // Draw sources and forces
    velocityRender = world.RecordVelocity({force1, force2}, Vortex::Fluid::VelocityOp::Set);
    densityRender = density->Record({source1, source2});

    // Draw rigid body
    body.mRigidbody.SetTransform({100.0f, 100.0f}, -45.0f);

    Vortex::Renderer::ColorBlendState blendState;
    blendState.ColorBlend.setBlendEnable(true)
        .setAlphaBlendOp(vk::BlendOp::eAdd)
        .setColorBlendOp(vk::BlendOp::eAdd)
        .setSrcColorBlendFactor(vk::BlendFactor::eSrcAlpha)
        .setSrcAlphaBlendFactor(vk::BlendFactor::eOne)
        .setDstColorBlendFactor(vk::BlendFactor::eOneMinusSrcAlpha)
        .setDstAlphaBlendFactor(vk::BlendFactor::eZero);

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

  b2World rWorld;

  Box2DSolver solver;
  RectangleRigidbody body;
};
