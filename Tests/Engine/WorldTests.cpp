//
//  WorldTests.cpp
//  Vortex2D
//

#include <gtest/gtest.h>
#include "VariationalHelpers.h"
#include <Vortex2D/Engine/World.h>
#include <Vortex2D/Engine/LinearSolver/IncompletePoisson.h>

#include <cmath>

using namespace Vortex2D::Renderer;
using namespace Vortex2D::Fluid;

extern Device* device;

void PrintParticles(const glm::ivec2& size, World& world)
{
    Buffer<Particle> particles(*device, 8*size.x*size.y, VMA_MEMORY_USAGE_CPU_ONLY);
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

    world.Count().Record({fluidArea}).Submit();

    // Draw gravity
    Vortex2D::Renderer::Rectangle gravity(*device, size.Size, {0.0f, -0.01f, 0.0f, 0.0f});

    auto gravityRender = world.Velocity().Record({gravity});

    // Step
    for (int i = 0; i < 10; i++)
    {
      gravityRender.Submit();
      world.SolveDynamic();
    }

    // wait to finish
    device->Handle().waitIdle();

    // Verify particle velocity
    Buffer<Particle> particles(*device, 8*size.Size.x*size.Size.y, VMA_MEMORY_USAGE_CPU_ONLY);
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

TEST(WorldTests, ObstacleVelocity)
{
    glm::ivec2 size(20);

    Buffer<glm::ivec2> valid(*device, size.x*size.y);
    LinearSolver::Data data(*device, size, VMA_MEMORY_USAGE_CPU_ONLY);

    IncompletePoisson preconditioner(*device, size);
    ConjugateGradient linearSolver(*device, size, preconditioner);

    linearSolver.Init(data.Diagonal, data.Lower, data.B, data.X);

    LevelSet fluidLevelSet(*device, size);
    LevelSet obstacleLevelSet(*device, size);

    RenderTexture velocity(*device, size.x, size.y, vk::Format::eR32G32Sfloat);
    RenderTexture boundariesVelocity(*device, size.x, size.y, vk::Format::eR32G32Sfloat);

    Pressure projection(*device, 0.01f, size, data, velocity, obstacleLevelSet, fluidLevelSet, boundariesVelocity, valid);

    // Draw fluid level set
    Clear clear({1.0f, 0.0f, 0.0f, 0.0f});
    Rectangle liquidArea(*device, {18.0f, 18.0f}, {-1.0f, 0.0f, 0.0f, 0.0f});
    liquidArea.Position = glm::vec2(1.0f);

    fluidLevelSet.Record({clear, liquidArea}).Submit();

    // Draw solid level set
    Rectangle solidArea(*device, {10.0f, 10.0f}, {-1.0f, 0.0f, 0.0f, 0.0f});
    solidArea.Position = glm::vec2(5.0f);
    solidArea.Rotation = 45.0f;

    obstacleLevelSet.Record({clear, solidArea}).Submit();

    // Draw solid velocity
    Rectangle solidVelocity(*device, {10.0f, 10.0f}, {-1.0f, 0.0f, 0.0f, 0.0f});
    solidVelocity.Position = glm::vec2(5.0f);
    solidVelocity.Rotation = 45.0f;

    boundariesVelocity.Record({solidVelocity}).Submit();

    // Solve
    LinearSolver::Parameters params(300, 1e-3f);
    projection.BuildLinearEquation();
    linearSolver.Solve(params);

    device->Handle().waitIdle();

    // Verify it got solved
    ASSERT_LE(params.OutError, 1e-3f);
}
