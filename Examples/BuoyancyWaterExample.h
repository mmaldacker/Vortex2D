//
//  HydrostaticWaterExample.h
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

class HydrostaticWaterExample : public Runner
{
  const float gravityForce = 100.0f;

public:
  HydrostaticWaterExample(Vortex::Renderer::Device& device, const glm::ivec2& size, float dt)
      : world(device, size, dt, 2, Vortex::Fluid::Velocity::InterpolationMode::Linear)
      , rWorld(b2Vec2(0.0f, gravityForce))
      , solver(rWorld)
      , circle1(device,
                size,
                rWorld,
                b2_dynamicBody,
                Vortex::Fluid::RigidBody::Type::eStrong,
                10.0f,
                0.4f)
      , circle2(device,
                size,
                rWorld,
                b2_dynamicBody,
                Vortex::Fluid::RigidBody::Type::eStrong,
                10.0f,
                0.7f)
      , circle3(device,
                size,
                rWorld,
                b2_dynamicBody,
                Vortex::Fluid::RigidBody::Type::eStrong,
                10.0f,
                1.1f)
      , left(device,
             size,
             rWorld,
             b2_staticBody,
             Vortex::Fluid::RigidBody::Type::eStatic,
             {5.0f, 125.0f})
      , right(device,
              size,
              rWorld,
              b2_staticBody,
              Vortex::Fluid::RigidBody::Type::eStatic,
              {5.0f, 125.0f})
      , bottom(device,
               size,
               rWorld,
               b2_staticBody,
               Vortex::Fluid::RigidBody::Type::eStatic,
               {250.0f, 5.0f})
  {
    world.AttachRigidBodySolver(solver);
    world.AddRigidbody(circle1.mRigidbody);
    world.AddRigidbody(circle2.mRigidbody);
    world.AddRigidbody(circle3.mRigidbody);
    world.AddRigidbody(left.mRigidbody);
    world.AddRigidbody(right.mRigidbody);
    world.AddRigidbody(bottom.mRigidbody);
  }

  void Init(Vortex::Renderer::Device& device, Vortex::Renderer::RenderTarget& renderTarget) override
  {
    auto gravity = std::make_shared<Vortex::Renderer::Rectangle>(device, glm::vec2(256.0f, 256.0f));
    gravity->Colour = glm::vec4(0.0f, dt * gravityForce, 0.0f, 0.0f);

    // Add particles
    auto fluid =
        std::make_shared<Vortex::Renderer::IntRectangle>(device, glm::vec2{245.0f, 100.0f});
    fluid->Position = {8.0f, 150.0f};
    fluid->Colour = glm::vec4(4);

    world.RecordParticleCount({fluid}).Submit().Wait();

    // Draw boundaries
    left.mRigidbody.SetTransform({3.0f, 250.0f}, 0.0f);
    right.mRigidbody.SetTransform({250.0f, 250.0f}, 0.0f);
    bottom.mRigidbody.SetTransform({3.0f, 250.0f}, 0.0f);

    // Add circles
    circle1.mRigidbody.SetTransform({50.0f, 100.0f}, 0.0f);
    circle2.mRigidbody.SetTransform({125.0f, 100.0f}, 0.0f);
    circle3.mRigidbody.SetTransform({200.0f, 100.0f}, 0.0f);

    // Set gravity
    velocityRender = world.RecordVelocity({gravity}, Vortex::Fluid::VelocityOp::Add);

    auto solidPhi = world.MakeSolidDistanceField();
    auto liquidPhi = world.MakeLiquidDistanceField();
    solidPhi->Colour = green;
    liquidPhi->Colour = blue;

    Vortex::Renderer::ColorBlendState blendState(Vortex::Renderer::BlendFactor::SrcAlpha,
                                                 Vortex::Renderer::BlendFactor::OneMinusSrcAlpha,
                                                 Vortex::Renderer::BlendOp::Add,
                                                 Vortex::Renderer::BlendFactor::One,
                                                 Vortex::Renderer::BlendFactor::Zero,
                                                 Vortex::Renderer::BlendOp::Add);

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
  float dt;
  Vortex::Fluid::WaterWorld world;
  Vortex::Renderer::RenderCommand velocityRender, windowRender;

  b2World rWorld;

  Box2DSolver solver;
  CircleRigidbody circle1, circle2, circle3;
  RectangleRigidbody left, right, bottom;
};
