//
//  WorldTests.cpp
//  Vortex2D
//

#include <gtest/gtest.h>
#include "VariationalHelpers.h"
#include "Verify.h"
#include <Vortex2D/Engine/World.h>
#include <Vortex2D/Engine/Rigidbody.h>
#include <Vortex2D/Engine/Boundaries.h>
#include <Vortex2D/Engine/Density.h>
#include <Vortex2D/Engine/Cfl.h>

#include <cmath>
#include <random>

using namespace Vortex2D;

extern Renderer::Device* device;

float PressureRigidbody_VelocityTest(float scale)
{
    float dt = 0.01f;
    glm::vec2 size(1024.0f, 1024.0f);

    Fluid::Dimensions dimensions(size, scale);
    Fluid::SmokeWorld world(*device, dimensions, dt);

    Fluid::Density density(*device, size, vk::Format::eR8G8B8A8Unorm);
    world.FieldBind(density);

    Renderer::Clear fluidClear({-1.0f, 0.0f, 0.0f, 0.0f});
    world.RecordLiquidPhi({fluidClear}).Submit();

    glm::vec2 rectangleSize(32.0f, 256.0f);
    Fluid::Rectangle rectangle(*device, rectangleSize);

    auto* rigidbody = world.CreateRigidbody(Fluid::RigidBody::Type::eStrong, rectangle, rectangleSize / glm::vec2(2.0f));
    rigidbody->Anchor = rectangleSize / glm::vec2(2.0f);
    rigidbody->Position = size / glm::vec2(2.0f);
    rigidbody->UpdatePosition();

    Renderer::Rectangle velocity(*device, size);
    velocity.Colour = {10.0f / (scale * size.x), 0.0f, 0.0f, 0.0f};

    world.RecordVelocity({velocity}).Submit();
    world.Solve();

    device->Handle().waitIdle();

    auto forces = rigidbody->GetForces();
    float force = forces.velocity.x;
    std::cout << "Scale " << scale << " Force (" << force << ")" << std::endl;

    // TODO relatively big error
    EXPECT_NEAR(forces.angular_velocity, 0.0f, 1.0f);
    EXPECT_NEAR(forces.velocity.y, 0.0f, 1.0f);

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
    for (int i = 0; i < 4; i++) avg += x[i];
    avg /= 4.0f;

    float var = 0.0f;
    for (int i = 0; i < 4; i++) var += (x[i] - avg) * (x[i] - avg);
    var /= 4.0f;

    std::cout << "Std deviation: " << std::sqrt(var) << std::endl;
}

float PressureRigidbody_RotationTest(float scale)
{
    float dt = 0.01f;
    glm::vec2 size(1024.0f, 1024.0f);

    Fluid::Dimensions dimensions(size, scale);
    Fluid::SmokeWorld world(*device, dimensions, dt);

    Fluid::Density density(*device, size, vk::Format::eR8G8B8A8Unorm);
    world.FieldBind(density);

    Renderer::Clear fluidClear({-1.0f, 0.0f, 0.0f, 0.0f});
    world.RecordLiquidPhi({fluidClear}).Submit();

    glm::vec2 rectangleSize(32.0f, 256.0f);
    Fluid::Rectangle rectangle(*device, rectangleSize);

    auto* rigidbody = world.CreateRigidbody(Fluid::RigidBody::Type::eStrong, rectangle, rectangleSize / glm::vec2(2.0f));
    rigidbody->Anchor = rectangleSize / glm::vec2(2.0f);
    rigidbody->Position = size / glm::vec2(2.0f);
    rigidbody->UpdatePosition();

    Renderer::Rectangle velocityUp(*device, glm::vec2(size.x, size.y / 2.0f));
    velocityUp.Position = {0.0f, 0.0f};
    velocityUp.Colour = {10.0f / (scale * size.x), 0.0f, 0.0f, 0.0f};

    Renderer::Rectangle velocityDown(*device, glm::vec2(size.x, size.y / 2.0f));
    velocityDown.Position = {0.0f, size.x / 2.0f};
    velocityDown.Colour = {-10.0f / (scale * size.x), 0.0f, 0.0f, 0.0f};

    world.RecordVelocity({velocityUp, velocityDown}).Submit();
    world.Solve();

    device->Handle().waitIdle();

    auto forces = rigidbody->GetForces();
    float force = forces.angular_velocity;
    std::cout << "Scale " << scale << " Force (" << force << ")" << std::endl;

    // TODO relatively big error
    EXPECT_NEAR(forces.velocity.x, 0.0f, 10.0f);
    EXPECT_NEAR(forces.velocity.y, 0.0f, 10.0f);

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
    for (int i = 0; i < 4; i++) avg += x[i];
    avg /= 4.0f;

    float var = 0.0f;
    for (int i = 0; i < 4; i++) var += (x[i] - avg) * (x[i] - avg);
    var /= 4.0f;

    std::cout << "Std deviation: " << std::sqrt(var) << std::endl;
}

TEST(CflTets, Max)
{
    glm::ivec2 size(50);

    Fluid::Velocity velocity(*device, size);
    Fluid::Cfl cfl(*device, size, velocity);

    Renderer::Texture input(*device, size.x, size.y, vk::Format::eR32G32Sfloat, VMA_MEMORY_USAGE_CPU_ONLY);

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(-1.0, -1.0);

    std::vector<glm::vec2> velocityData(size.x * size.y, glm::vec2(0.0f));
    for (int i = 0; i < size.x; i++)
    {
        for (int j = 0; j < size.y; j++)
        {
            std::size_t index = i + size.x * j;
            velocityData[index].x = dis(gen);
            velocityData[index].y = dis(gen);
        }
    }

    velocityData[12].x = -1.0;

    input.CopyFrom(velocityData);
    Renderer::ExecuteCommand(*device, [&](vk::CommandBuffer commandBuffer)
    {
        velocity.CopyFrom(commandBuffer, input);
    });

    cfl.Compute();
    EXPECT_EQ(1.0f / size.x, cfl.Get());
}
