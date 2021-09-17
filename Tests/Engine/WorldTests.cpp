//
//  WorldTests.cpp
//  Vortex
//

#include <Vortex/Engine/Boundaries.h>
#include <Vortex/Engine/Cfl.h>
#include <Vortex/Engine/Density.h>
#include <Vortex/Engine/Rigidbody.h>
#include <Vortex/Engine/World.h>
#include <gtest/gtest.h>
#include "VariationalHelpers.h"
#include "Verify.h"

#include <cmath>
#include <iostream>
#include <random>

using namespace Vortex;

extern Renderer::Device* device;

float PressureRigidbody_VelocityTest(float scale)
{
  float dt = 0.01f;
  glm::vec2 size(1024.0f, 1024.0f);
  size /= scale;

  Fluid::SmokeWorld world(*device, size, dt, Fluid::Velocity::InterpolationMode::Cubic);

  Fluid::Density density(*device, size, vk::Format::eR8G8B8A8Unorm);
  world.FieldBind(density);

  auto fluidClear = std::make_shared<Renderer::Clear>(glm::vec4{-1.0f, 0.0f, 0.0f, 0.0f});
  world.RecordLiquidPhi({fluidClear}).Submit();

  glm::vec2 rectangleSize(32.0f, 256.0f);
  rectangleSize /= scale;

  auto rectangle = std::make_shared<Fluid::Rectangle>(*device, rectangleSize);

  float mass = rectangleSize.x * rectangleSize.y / (scale * scale);
  float inertia = rectangleSize.x * rectangleSize.y *
                  (rectangleSize.x * rectangleSize.x + rectangleSize.y * rectangleSize.y) /
                  (12.0f * std::pow(scale, 4.0f));

  auto rigidbody =
      std::make_shared<Fluid::RigidBody>(*device, size, rectangle, Fluid::RigidBody::Type::eWeak);
  rigidbody->SetMassData(mass, inertia);

  world.AddRigidbody(*rigidbody);
  rigidbody->Anchor = rectangleSize / glm::vec2(2.0f);
  rigidbody->Position = size / glm::vec2(2.0f);

  auto velocity = std::make_shared<Renderer::Rectangle>(*device, size);
  velocity->Colour = {10.0f / scale, 0.0f, 0.0f, 0.0f};

  world.RecordVelocity({velocity}, Fluid::VelocityOp::Set).Submit();

  auto params = Fluid::IterativeParams(1e-5f);
  world.Step(params);

  device->Handle().waitIdle();

  auto forces = rigidbody->GetForces();
  float force = forces.velocity.x / (mass * scale);
  std::cout << "Scale " << scale << " Mass " << mass << " Scaled Force (" << force << ")"
            << std::endl;

  EXPECT_NE(force, 0.0f);
  EXPECT_NEAR(forces.angular_velocity / (inertia * std::pow(scale, 4.0f)), 0.0f, 0.1f);
  EXPECT_NEAR(forces.velocity.y / (mass * scale), 0.0f, 0.1f);

  device->Handle().waitIdle();

  return force;
}

TEST(WorldTests, PressureRigidbody_Velocity)
{
  float x[4];
  x[0] = PressureRigidbody_VelocityTest(2.0f);
  x[1] = PressureRigidbody_VelocityTest(4.0f);
  x[2] = PressureRigidbody_VelocityTest(8.0f);
  x[3] = PressureRigidbody_VelocityTest(16.0f);

  float avg = 0.0f;
  for (int i = 0; i < 4; i++)
    avg += x[i];
  avg /= 4.0f;

  float var = 0.0f;
  for (int i = 0; i < 4; i++)
    var += (x[i] - avg) * (x[i] - avg);
  var /= 4.0f;

  std::cout << "Std deviation: " << std::sqrt(var) << std::endl;
}

float PressureRigidbody_RotationTest(float scale)
{
  float dt = 0.01f;
  glm::vec2 size(1024.0f, 1024.0f);
  size /= scale;

  Fluid::SmokeWorld world(*device, size, dt, Fluid::Velocity::InterpolationMode::Cubic);

  Fluid::Density density(*device, size, vk::Format::eR8G8B8A8Unorm);
  world.FieldBind(density);

  auto fluidClear = std::make_shared<Renderer::Clear>(glm::vec4{-1.0f, 0.0f, 0.0f, 0.0f});
  world.RecordLiquidPhi({fluidClear}).Submit();

  glm::vec2 rectangleSize(32.0f, 256.0f);
  rectangleSize /= scale;

  auto rectangle = std::make_shared<Fluid::Rectangle>(*device, rectangleSize);

  float mass = rectangleSize.x * rectangleSize.y / (scale * scale);
  float inertia = rectangleSize.x * rectangleSize.y *
                  (rectangleSize.x * rectangleSize.x + rectangleSize.y * rectangleSize.y) /
                  (12.0f * std::pow(scale, 4.0f));

  auto rigidbody =
      std::make_shared<Fluid::RigidBody>(*device, size, rectangle, Fluid::RigidBody::Type::eWeak);
  rigidbody->SetMassData(mass, inertia);
  world.AddRigidbody(*rigidbody);
  rigidbody->Anchor = rectangleSize / glm::vec2(2.0f);
  rigidbody->Position = size / glm::vec2(2.0f);

  auto velocityUp =
      std::make_shared<Renderer::Rectangle>(*device, glm::vec2(size.x, size.y / 2.0f));
  velocityUp->Position = {0.0f, 0.0f};
  velocityUp->Colour = {10.0f / scale, 0.0f, 0.0f, 0.0f};

  auto velocityDown =
      std::make_shared<Renderer::Rectangle>(*device, glm::vec2(size.x, size.y / 2.0f));
  velocityDown->Position = {0.0f, size.x / 2.0f};
  velocityDown->Colour = {-10.0f / scale, 0.0f, 0.0f, 0.0f};

  world.RecordVelocity({velocityUp, velocityDown}, Fluid::VelocityOp::Set).Submit();

  auto params = Fluid::IterativeParams(1e-5f);
  world.Step(params);

  device->Handle().waitIdle();

  auto forces = rigidbody->GetForces();
  float force = forces.angular_velocity / (inertia * std::pow(scale, 4.0f));
  std::cout << "Scale " << scale << " Inertia " << inertia << " Scaled Torque (" << force << ")"
            << std::endl;

  EXPECT_NE(force, 0.0f);
  EXPECT_NEAR(forces.velocity.x / (mass * scale), 0.0f, 0.1f);
  EXPECT_NEAR(forces.velocity.y / (mass * scale), 0.0f, 0.1f);

  device->Handle().waitIdle();

  return force;
}

TEST(WorldTests, PressureRigidbody_Rotation)
{
  float x[4];
  x[0] = PressureRigidbody_RotationTest(2.0f);
  x[1] = PressureRigidbody_RotationTest(4.0f);
  x[2] = PressureRigidbody_RotationTest(8.0f);
  x[3] = PressureRigidbody_RotationTest(16.0f);

  float avg = 0.0f;
  for (int i = 0; i < 4; i++)
    avg += x[i];
  avg /= 4.0f;

  float var = 0.0f;
  for (int i = 0; i < 4; i++)
    var += (x[i] - avg) * (x[i] - avg);
  var /= 4.0f;

  std::cout << "Std deviation: " << std::sqrt(var) << std::endl;
}

TEST(WorldTests, Velocity)
{
  float dt = 0.01f;
  glm::vec2 size(256.0f, 256.0f);

  Fluid::SmokeWorld world(*device, size, dt, Fluid::Velocity::InterpolationMode::Cubic);

  auto fluidClear = std::make_shared<Renderer::Clear>(glm::vec4{-1.0f, 0.0f, 0.0f, 0.0f});
  world.RecordLiquidPhi({fluidClear}).Submit();

  auto velocity = std::make_shared<Renderer::Rectangle>(*device, size);
  velocity->Colour = {-10.0f, -10.0f, 0.0f, 0.0f};

  world.RecordVelocity({velocity}, Fluid::VelocityOp::Set).Submit();

  auto params = Fluid::IterativeParams(1e-5f);
  world.Step(params);

  device->Handle().waitIdle();

  float value = 10.0f / size.x;
  std::vector<glm::vec2> velocityData(size.x * size.y, {-value, -value});

  CheckVelocity(*device, size, world.GetVelocity(), velocityData);
}

TEST(CflTets, Max)
{
  glm::ivec2 size(50);

  Fluid::Velocity velocity(*device, size);
  Fluid::Cfl cfl(*device, size, velocity);

  Renderer::Texture input(
      *device, size.x, size.y, vk::Format::eR32G32Sfloat, VMA_MEMORY_USAGE_CPU_ONLY);

  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_real_distribution<> dis(-1.0, 1.0);
  float max = -1.0f;

  std::vector<glm::vec2> velocityData(size.x * size.y, glm::vec2(0.0f));
  for (int i = 0; i < size.x; i++)
  {
    for (int j = 0; j < size.y; j++)
    {
      std::size_t index = i + size.x * j;
      velocityData[index].x = dis(gen);
      velocityData[index].y = dis(gen);

      max = std::max(std::max(velocityData[index].x, velocityData[index].y), max);
    }
  }

  input.CopyFrom(velocityData);
  device->Execute([&](vk::CommandBuffer commandBuffer)
                  { velocity.CopyFrom(commandBuffer, input); });

  cfl.Compute();
  EXPECT_NEAR(1.0f / (max * size.x), cfl.Get(), 1e-4f);
}
