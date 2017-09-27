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

TEST(WorldTests, Velocity)
{
    Dimensions size(glm::ivec2(20), 1.0f);
    Vortex2D::Fluid::World world(*device, size, 0.01f);

    // Add particles
    Vortex2D::Renderer::IntRectangle fluidArea(*device, glm::vec2(5.0f), glm::ivec4(4));
    fluidArea.Position = {5.0f, 5.0f};

    fluidArea.Initialize(world.Count());
    fluidArea.Update(world.Count().Orth, size.InvScale);

    world.Count().Record([&](vk::CommandBuffer commandBuffer)
    {
        fluidArea.Draw(commandBuffer, {world.Count()});
    });
    world.Count().Submit();
    device->Handle().waitIdle();

    // Draw obstacle
    Vortex2D::Renderer::Rectangle obstacle(*device, {10.0f, 5.0f}, glm::vec4(-1.0f));
    obstacle.Position = {5.0f, 7.0f};
    obstacle.Initialize(world.SolidPhi());
    obstacle.Update(world.SolidPhi().Orth, size.InvScale);
    world.SolidPhi().Record([&](vk::CommandBuffer commandBuffer)
    {
        Vortex2D::Renderer::Clear(size.Size.x, size.Size.y, {-1.0f, 0.0f, 0.0f, 0.0f}).Draw(commandBuffer);
        obstacle.Draw(commandBuffer, world.SolidPhi());
    });
    world.SolidPhi().Submit();

    world.SolidPhi().Reinitialise();
    device->Handle().waitIdle();

    // Draw gravity
    Vortex2D::Renderer::Rectangle gravity(*device, size.Size, {0.0f, -0.5f, 0.0f, 0.0f});

    gravity.Initialize(world.Velocity());
    gravity.Update(world.Velocity().Orth, {});

    world.Velocity().Record([&](vk::CommandBuffer commandBuffer)
    {
        gravity.Draw(commandBuffer, world.Velocity());
    });
    world.Velocity().Submit();

    // Step
    world.SolveDynamic();
    device->Handle().waitIdle();

    // Verify
    Texture output(*device, size.Size.x, size.Size.y, vk::Format::eR32G32Sfloat, true);
    ExecuteCommand(*device, [&](vk::CommandBuffer commandBuffer)
    {
        output.CopyFrom(commandBuffer, world.Velocity());
    });

    PrintVelocity(size.Size, output);

    Buffer particles(*device, vk::BufferUsageFlagBits::eStorageBuffer, true, 8*size.Size.x*size.Size.y*sizeof(Particle));
    std::vector<Particle> particleData(8*size.Size.x*size.Size.y);
    ExecuteCommand(*device, [&](vk::CommandBuffer commandBuffer)
    {
       particles.CopyFrom(commandBuffer, world.Particles());
    });

    int particleCount = world.Count().GetCount();
    std::cout << "Particle Count: " << particleCount << std::endl;

    particles.CopyTo(particleData);
    for (int i = 0; i < particleCount; i++)
    {
        std::cout << "(" << particleData[i].Position.x << "," << particleData[i].Position.y << ")"
                  << "(" << particleData[i].Velocity.x << "," << particleData[i].Velocity.y << ")"
                  << std::endl;
    }
}
