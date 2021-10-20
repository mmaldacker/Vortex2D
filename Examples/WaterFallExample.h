//
//  WaterFallExample.h
//  Vortex
//

#pragma once

#include <Vortex/Vortex.h>

#include "Rigidbody.h"
#include "Runner.h"

#include <functional>
#include <memory>
#include <vector>

extern glm::vec4 green;
extern glm::vec4 blue;

class WaterFallExample : public Runner
{
  const float gravityForce = 100.0f;

public:
  WaterFallExample(Vortex::Renderer::Device& device, const glm::ivec2& size, float dt)
      : dt(dt)
      , world(device, size, dt, 2, Vortex::Fluid::Velocity::InterpolationMode::Linear)
      , rWorld(b2Vec2(0.0f, gravityForce))
      , solver(rWorld)
      , circle(device, size, rWorld, b2_dynamicBody, Vortex::Fluid::RigidBody::Type::eStrong, 10.0f)
      , box(device,
            size,
            rWorld,
            b2_dynamicBody,
            Vortex::Fluid::RigidBody::Type::eStrong,
            {15.0f, 15.0f})
      , left(device,
             size,
             rWorld,
             b2_staticBody,
             Vortex::Fluid::RigidBody::Type::eStatic,
             {50.0f, 5.0f})
      , right(device,
              size,
              rWorld,
              b2_staticBody,
              Vortex::Fluid::RigidBody::Type::eStatic,
              {50.0f, 5.0f})
      , bottom(device,
               size,
               rWorld,
               b2_staticBody,
               Vortex::Fluid::RigidBody::Type::eStatic,
               {250.0f, 5.0f})
  {
    world.AttachRigidBodySolver(solver);
    world.AddRigidbody(circle.mRigidbody);
    world.AddRigidbody(box.mRigidbody);
    world.AddRigidbody(left.mRigidbody);
    world.AddRigidbody(right.mRigidbody);
    world.AddRigidbody(bottom.mRigidbody);
  }

  void Init(Vortex::Renderer::Device& device,
            Vortex::Renderer::RenderTarget& renderTarget) override
  {
    auto waterSource =
        std::make_shared<Vortex::Renderer::IntRectangle>(device, glm::vec2{10.0f, 10.0f});
    auto waterForce =
        std::make_shared<Vortex::Renderer::Rectangle>(device, glm::vec2{10.0f, 10.0f});
    auto gravity = std::make_shared<Vortex::Renderer::Rectangle>(device, glm::vec2{256.0f, 256.0f});

    gravity->Colour = glm::vec4(0.0f, dt * gravityForce, 0.0f, 0.0f);

    // Add particles
    waterSource->Position = {5.0f, 25.0f};
    waterSource->Colour = glm::vec4(4);

    // Add force
    waterForce->Position = {5.0f, 25.0f};
    waterForce->Colour = glm::vec4(10.0f, 0.0f, 0.0f, 0.0f);

    sourceRender = world.RecordParticleCount({waterSource});

    // Draw boundaries
    left.mRigidbody.SetTransform({50.0f, 80.0f}, 20.0f);
    right.mRigidbody.SetTransform({175.0f, 125.0f}, -20.0f);
    bottom.mRigidbody.SetTransform({5.0f, 250.0f}, 0.0f);

    // Add circles
    circle.mRigidbody.SetTransform({50.0f, 50.0f}, 0.0f);
    box.mRigidbody.SetTransform({75.0f, 50.0f}, 0.0f);

    // Set gravity
    velocityRender = world.RecordVelocity({gravity, waterForce}, Vortex::Fluid::VelocityOp::Add);

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
    sourceRender.Submit();
    world.SubmitVelocity(velocityRender);
    auto params = Vortex::Fluid::FixedParams(12);
    world.Step(params);
    windowRender.Submit();
  }

private:
  float dt;
  Vortex::Fluid::WaterWorld world;
  Vortex::Renderer::RenderCommand sourceRender, velocityRender, windowRender;

  b2World rWorld;

  Box2DSolver solver;
  CircleRigidbody circle;
  RectangleRigidbody box;
  RectangleRigidbody left, right, bottom;
};
