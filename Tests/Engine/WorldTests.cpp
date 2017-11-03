//
//  WorldTests.cpp
//  Vortex2D
//

#include <gtest/gtest.h>
#include "VariationalHelpers.h"
#include <Vortex2D/Engine/World.h>

#include <cmath>

using namespace Vortex2D::Renderer;
using namespace Vortex2D::Fluid;

extern Device* device;

void PrintParticles(const glm::ivec2& size, World& world)
{
    Buffer<Particle> particles(*device, 8*size.x*size.y, true);
    std::vector<Particle> particleData(8*size.x*size.y);

    ExecuteCommand(*device, [&](vk::CommandBuffer commandBuffer)
    {
       particles.CopyFrom(commandBuffer, world.Particles());
    });

    CopyTo(particles, particleData);

    int particleCount = world.Count().GetCount();
    for (int i = 0; i < particleCount; i++)
    {
        std::cout << "(" << particleData[i].Position.x << "," << particleData[i].Position.y << ")"
                  << "(" << particleData[i].Velocity.x << "," << particleData[i].Velocity.y << ")";
    }
    std::cout << std::endl;
}

TEST(WorldTests, Velocity)
{
    Dimensions size(glm::ivec2(20), 1.0f);
    Vortex2D::Fluid::World world(*device, size, 0.01f);

    // Add particles
    Vortex2D::Renderer::IntRectangle fluidArea(*device, glm::vec2(1.0f), glm::ivec4(4));
    fluidArea.Position = {10.0f, 2.0f};

    world.Count().Record({fluidArea});
    world.Count().Submit();
    device->Handle().waitIdle();

    // Draw gravity
    Vortex2D::Renderer::Rectangle gravity(*device, size.Size, {0.0f, -0.01f, 0.0f, 0.0f});

    world.Velocity().Record({gravity});

    // Step
    for (int i = 0; i < 10; i++)
    {
      world.Velocity().Submit();
      world.SolveDynamic();
      device->Handle().waitIdle();
    }

    // Verify particle velocity
    Buffer<Particle> particles(*device, 8*size.Size.x*size.Size.y, true);
    std::vector<Particle> particleData(8*size.Size.x*size.Size.y);

    ExecuteCommand(*device, [&](vk::CommandBuffer commandBuffer)
    {
       particles.CopyFrom(commandBuffer, world.Particles());
    });

    CopyTo(particles, particleData);

    glm::vec2 expectedVelocity(0.0f, -0.1f);

    int particleCount = world.Count().GetCount();
    for (int i = 0; i < particleCount; i++)
    {
        EXPECT_NEAR(expectedVelocity.x, particleData[i].Velocity.x, 1e-5f);
        EXPECT_NEAR(expectedVelocity.y, particleData[i].Velocity.y, 1e-5f);
    }
}
