//
//  PressureTests.cpp
//  Vortex2D
//

#include "VariationalHelpers.h"
#include "Verify.h"
#include "Renderer/ShapeDrawer.h"

#include <Vortex2D/Engine/Pressure.h>
#include <iostream>

using namespace Vortex2D::Renderer;
using namespace Vortex2D::Fluid;
using ::testing::NiceMock;
using ::testing::Invoke;
using ::testing::_;

extern Device* device;

void PrintDiv(const glm::ivec2& size, Buffer<float>& buffer)
{
    std::vector<float> pixels(size.x * size.y);
    CopyTo(buffer, pixels);

    for (std::size_t j = 0; j < size.y; j++)
    {
        for (std::size_t i = 0; i < size.x; i++)
        {
            std::size_t index = i + size.x * j;
            std::cout << "(" <<  pixels[index] << ")";
        }
        std::cout << std::endl;
    }
}

void PrintDiagonal(const glm::ivec2& size, FluidSim& sim)
{
    for (std::size_t j = 0; j < size.y; j++)
    {
        for (std::size_t i = 0; i < size.x; i++)
        {
            std::size_t index = i + size.x * j;
            std::cout << "(" <<  sim.matrix(index, index) << ")";
        }
        std::cout << std::endl;
    }
}

void CheckDiagonal(const glm::ivec2& size, Buffer<float>& buffer, FluidSim& sim, float error = 1e-6)
{
    std::vector<float> pixels(size.x * size.y);
    CopyTo(buffer, pixels);

    for (std::size_t i = 0; i < size.x; i++)
    {
        for (std::size_t j = 0; j < size.y; j++)
        {
            std::size_t index = i + size.x * j;
            EXPECT_NEAR(sim.matrix(index, index), pixels[index], error);
        }
    }
}

void CheckWeights(const glm::ivec2& size, Buffer<glm::vec2>& buffer, FluidSim& sim, float error = 1e-6)
{
    std::vector<glm::vec2> pixels(size.x * size.y);
    CopyTo(buffer, pixels);

    for (std::size_t i = 1; i < size.x - 1; i++)
    {
        for (std::size_t j = 1; j < size.y - 1; j++)
        {
            std::size_t index = i + size.x * j;
            EXPECT_NEAR(sim.matrix(index - 1, index), pixels[index].x, error);
            EXPECT_NEAR(sim.matrix(index, index - size.x), pixels[index].y, error);
        }
    }
}

void CheckDiv(const glm::ivec2& size, Buffer<float>& buffer, FluidSim& sim, float error = 1e-6)
{
    std::vector<float> pixels(size.x * size.y);
    CopyTo(buffer, pixels);

    for (std::size_t i = 0; i < size.x; i++)
    {
        for (std::size_t j = 0; j < size.y; j++)
        {
            std::size_t index = i + size.x * j;
            EXPECT_NEAR(sim.rhs[index], pixels[index], error);
        }
    }
}

TEST(PressureTest, LinearEquationSetup_Simple)
{
    glm::ivec2 size(50);

    FluidSim sim;
    sim.initialize(1.0f, size.x, size.y);
    sim.set_boundary(boundary_phi);

    AddParticles(size, sim, boundary_phi);

    sim.add_force(0.01f);

    Texture velocity(*device, size.x, size.y, vk::Format::eR32G32Sfloat);
    Texture solidPhi(*device, size.x, size.y, vk::Format::eR32Sfloat);
    Texture liquidPhi(*device, size.x, size.y, vk::Format::eR32Sfloat);
    Texture solidVelocity(*device, size.x, size.y, vk::Format::eR32G32Sfloat);

    BuildInputs(*device, size, sim, velocity, solidPhi, liquidPhi);

    LinearSolver::Data data(*device, size, VMA_MEMORY_USAGE_CPU_ONLY);
    Buffer<glm::ivec2> valid(*device, size.x*size.y, VMA_MEMORY_USAGE_CPU_ONLY);

    Pressure pressure(*device, 0.01f, size, data, velocity, solidPhi, liquidPhi, solidVelocity, valid);

    pressure.BuildLinearEquation();
    device->Handle().waitIdle();

    CheckDiagonal(size, data.Diagonal, sim, 1e-3f); // FIXME can we reduce error tolerance?
    CheckWeights(size, data.Lower, sim, 1e-3f); // FIXME can we reduce error tolerance?
    CheckDiv(size, data.B, sim);
}

TEST(PressureTest, LinearEquationSetup_Complex)
{
    glm::ivec2 size(50);

    FluidSim sim;
    sim.initialize(1.0f, size.x, size.y);
    sim.set_boundary(boundary_phi);

    AddParticles(size, sim, complex_boundary_phi);

    sim.add_force(0.01f);

    Texture velocity(*device, size.x, size.y, vk::Format::eR32G32Sfloat);
    Texture solidPhi(*device, size.x, size.y, vk::Format::eR32Sfloat);
    Texture liquidPhi(*device, size.x, size.y, vk::Format::eR32Sfloat);
    Texture solidVelocity(*device, size.x, size.y, vk::Format::eR32G32Sfloat);

    BuildInputs(*device, size, sim, velocity, solidPhi, liquidPhi);

    LinearSolver::Data data(*device, size, VMA_MEMORY_USAGE_CPU_ONLY);
    Buffer<glm::ivec2> valid(*device, size.x*size.y, VMA_MEMORY_USAGE_CPU_ONLY);

    Pressure pressure(*device, 0.01f, size, data, velocity, solidPhi, liquidPhi, solidVelocity, valid);

    pressure.BuildLinearEquation();
    device->Handle().waitIdle();

    CheckDiagonal(size, data.Diagonal, sim, 1e-3f); // FIXME can we reduce error tolerance?
    CheckWeights(size, data.Lower, sim, 1e-3f); // FIXME can we reduce error tolerance?
    CheckDiv(size, data.B, sim);
}

TEST(PressureTest, ZeroDivs)
{
    glm::ivec2 size(50);

    Texture velocity(*device, size.x, size.y, vk::Format::eR32G32Sfloat);
    Texture solidPhi(*device, size.x, size.y, vk::Format::eR32Sfloat);
    Texture liquidPhi(*device, size.x, size.y, vk::Format::eR32Sfloat);
    Texture solidVelocity(*device, size.x, size.y, vk::Format::eR32G32Sfloat);

    Texture input(*device, size.x, size.y, vk::Format::eR32Sfloat, VMA_MEMORY_USAGE_CPU_ONLY);
    std::vector<float> inputData(size.x * size.y);
    DrawSquare(size.x, size.y, inputData, {10.0f, 10.0f}, {30.0f, 30.0f}, -1.0f);
    input.CopyFrom(inputData);
    ExecuteCommand(*device, [&](vk::CommandBuffer commandBuffer)
    {
        liquidPhi.CopyFrom(commandBuffer, input);
    });

    LinearSolver::Data data(*device, size, VMA_MEMORY_USAGE_CPU_ONLY);
    Buffer<glm::ivec2> valid(*device, size.x*size.y, VMA_MEMORY_USAGE_CPU_ONLY);

    Pressure pressure(*device, 0.01f, size, data, velocity, solidPhi, liquidPhi, solidVelocity, valid);

    pressure.BuildLinearEquation();
    device->Handle().waitIdle();

    std::vector<float> divOutputData(size.x*size.y);
    CopyTo(data.B, divOutputData);
    for (int i = 0; i < size.x; i++)
    {
        for (int j = 0; j < size.y; j++)
        {
            EXPECT_EQ(0.0f, divOutputData[i + size.x * j]);
        }
    }
}

TEST(PressureTest, Project_Simple)
{
    glm::ivec2 size(50);

    FluidSim sim;
    sim.initialize(1.0f, size.x, size.y);
    sim.set_boundary(boundary_phi);

    AddParticles(size, sim, boundary_phi);

    sim.add_force(0.01f);

    Texture velocity(*device, size.x, size.y, vk::Format::eR32G32Sfloat);
    Texture solidPhi(*device, size.x, size.y, vk::Format::eR32Sfloat);
    Texture liquidPhi(*device, size.x, size.y, vk::Format::eR32Sfloat);
    Texture solidVelocity(*device, size.x, size.y, vk::Format::eR32G32Sfloat);

    BuildInputs(*device, size, sim, velocity, solidPhi, liquidPhi);

    LinearSolver::Data data(*device, size, VMA_MEMORY_USAGE_CPU_ONLY);

    std::vector<float> computedPressureData(size.x*size.y, 0.0f);
    for (std::size_t i = 0; i < computedPressureData.size(); i++)
    {
        computedPressureData[i] = (float)sim.pressure[i];
    }
    CopyFrom(data.X, computedPressureData);

    Buffer<glm::ivec2> valid(*device, size.x*size.y, VMA_MEMORY_USAGE_CPU_ONLY);

    Pressure pressure(*device, 0.01f, size, data, velocity, solidPhi, liquidPhi, solidVelocity, valid);

    pressure.ApplyPressure();
    device->Handle().waitIdle();

    CheckVelocity(*device, size, velocity, sim);
    CheckValid(size, sim, valid);
}

TEST(PressureTest, Project_Complex)
{
    glm::ivec2 size(50);

    FluidSim sim;
    sim.initialize(1.0f, size.x, size.y);
    sim.set_boundary(boundary_phi);

    AddParticles(size, sim, boundary_phi);

    sim.add_force(0.01f);

    Texture velocity(*device, size.x, size.y, vk::Format::eR32G32Sfloat);
    Texture solidPhi(*device, size.x, size.y, vk::Format::eR32Sfloat);
    Texture liquidPhi(*device, size.x, size.y, vk::Format::eR32Sfloat);
    Texture solidVelocity(*device, size.x, size.y, vk::Format::eR32G32Sfloat);

    BuildInputs(*device, size, sim, velocity, solidPhi, liquidPhi);

    LinearSolver::Data data(*device, size, VMA_MEMORY_USAGE_CPU_ONLY);

    std::vector<float> computedPressureData(size.x*size.y, 0.0f);
    for (std::size_t i = 0; i < computedPressureData.size(); i++)
    {
        computedPressureData[i] = (float)sim.pressure[i];
    }
    CopyFrom(data.X, computedPressureData);

    Buffer<glm::ivec2> valid(*device, size.x*size.y, VMA_MEMORY_USAGE_CPU_ONLY);

    Pressure pressure(*device, 0.01f, size, data, velocity, solidPhi, liquidPhi, solidVelocity, valid);

    pressure.ApplyPressure();
    device->Handle().waitIdle();

    CheckVelocity(*device, size, velocity, sim);
    CheckValid(size, sim, valid);
}
