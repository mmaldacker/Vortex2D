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
    Buffer particles(*device, vk::BufferUsageFlagBits::eStorageBuffer, true, 8*size.x*size.y*sizeof(Particle));
    std::vector<Particle> particleData(8*size.x*size.y);
    
    ExecuteCommand(*device, [&](vk::CommandBuffer commandBuffer)
    {
       particles.CopyFrom(commandBuffer, world.Particles());
    });

    int particleCount = world.Count().GetCount();
    particles.CopyTo(particleData);
    for (int i = 0; i < particleCount; i++)
    {
        std::cout //<< "(" << particleData[i].Position.x << "," << particleData[i].Position.y << ")"
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

    fluidArea.Initialize(world.Count());
    fluidArea.Update(world.Count().Orth, size.InvScale);

    world.Count().Record([&](vk::CommandBuffer commandBuffer)
    {
        fluidArea.Draw(commandBuffer, {world.Count()});
    });
    world.Count().Submit();
    device->Handle().waitIdle();

    // Draw gravity
    Vortex2D::Renderer::Rectangle gravity(*device, size.Size, {0.0f, -0.01f, 0.0f, 0.0f});

    gravity.Initialize(world.Velocity());
    gravity.Update(world.Velocity().Orth, {});

    world.Velocity().Record([&](vk::CommandBuffer commandBuffer)
    {
        gravity.Draw(commandBuffer, world.Velocity());
    });

    // Step
    for (int i = 0; i < 10; i++)
    {
      world.Velocity().Submit();    
      world.SolveDynamic();
      device->Handle().waitIdle();
      
      PrintParticles(size.Size, world);
    }
}
