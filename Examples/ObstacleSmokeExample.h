//
//  ObstacleSmoke.h
//  Vortex
//

#pragma once

#include <Vortex/Vortex.h>

#include "Rigidbody.h"
#include "Runner.h"

#include <Box2D/Box2D.h>

#include <glm/trigonometric.hpp>

#include <functional>
#include <memory>
#include <vector>

extern glm::vec4 green;
extern glm::vec4 gray;

class ObstacleSmokeExample : public Runner
{
public:
  ObstacleSmokeExample(const Vortex::Renderer::Device& device, const glm::ivec2& size, float dt)
      : density(std::make_shared<Vortex::Fluid::Density>(device, size, vk::Format::eR8G8B8A8Unorm))
      , world(device, size, dt, Vortex::Fluid::Velocity::InterpolationMode::Linear)
      , rWorld({0.0f, 100.0f})
      , solver(rWorld)
      , body1(device,
              size,
              rWorld,
              b2_dynamicBody,
              Vortex::Fluid::RigidBody::Type::eStatic,
              {25.0f, 12.0f})
      , body2(device,
              size,
              rWorld,
              b2_dynamicBody,
              Vortex::Fluid::RigidBody::Type::eStatic,
              {12.0f, 12.0f})
      , bottom(device,
               size,
               rWorld,
               b2_staticBody,
               Vortex::Fluid::RigidBody::Type::eStatic,
               {125.0f, 5.0f})
  {
    world.FieldBind(*density);
    world.AttachRigidBodySolver(solver);
    world.AddRigidbody(body1.mRigidbody);
    world.AddRigidbody(body2.mRigidbody);
    world.AddRigidbody(bottom.mRigidbody);
  }

  void Init(const Vortex::Renderer::Device& device,
            Vortex::Renderer::RenderTarget& renderTarget) override
  {
    // Draw density
    auto source = std::make_shared<Vortex::Renderer::Rectangle>(device, glm::vec2{200, 100.0f});
    source->Position = {25.0f, 125.0f};
    source->Colour = gray;

    density->Record({source}).Submit().Wait();

    // Draw liquid boundaries
    auto clear = std::make_shared<Vortex::Renderer::Clear>(glm::vec4{1.0f, 0.0f, 0.0f, 0.0f});
    auto liquidArea =
        std::make_shared<Vortex::Renderer::Rectangle>(device, glm::vec2{250.0f, 250.0f});
    liquidArea->Colour = {-1.0f, 0.0f, 0.0f, 0.0f};
    liquidArea->Position = {3.0f, 3.0};

    world.RecordLiquidPhi({clear, liquidArea}).Submit().Wait();

    // Draw solid boundaries

    // First body
    body1.mRigidbody.SetTransform({50.0f, 50.0f}, 45.0f);
    body1.mRigidbody.mBody->ApplyAngularImpulse(5e5f, true);
    body1.mRigidbody.mBody->ApplyForceToCenter({1e5f, 0.0f}, true);

    // Second body
    body2.mRigidbody.SetTransform({200.0f, 80.0f}, 0.0f);
    body2.mRigidbody.mBody->ApplyAngularImpulse(-1e5f, true);
    body2.mRigidbody.mBody->ApplyForceToCenter({-1e5f, 0.0f}, true);

    // Bottom
    bottom.mRigidbody.SetTransform({128.0f, 256.5f}, 0.0f);

    auto solidPhi = world.MakeSolidDistanceField();
    solidPhi->Colour = green;

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
    auto params = Vortex::Fluid::FixedParams(12);
    world.Step(params);
    windowRender.Submit();
  }

private:
  std::shared_ptr<Vortex::Fluid::Density> density;
  Vortex::Fluid::SmokeWorld world;

  Vortex::Renderer::RenderCommand windowRender;

  b2World rWorld;

  Box2DSolver solver;
  RectangleRigidbody body1;
  RectangleRigidbody body2;
  RectangleRigidbody bottom;
};
