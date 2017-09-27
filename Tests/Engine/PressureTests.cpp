//
//  PressureTests.cpp
//  Vortex2D
//

#include "VariationalHelpers.h"
#include "Verify.h"

#include <Vortex2D/Engine/Pressure.h>
#include <iostream>

using namespace Vortex2D::Renderer;
using namespace Vortex2D::Fluid;
using ::testing::NiceMock;
using ::testing::Invoke;
using ::testing::_;

extern Device* device;

struct LinearSolverMock : LinearSolver
{
    MOCK_METHOD4(Init, void(Buffer& d, Buffer& l, Buffer& b, Buffer& pressure));
    MOCK_METHOD1(Solve, void(Parameters& params));
};

void PrintDiv(const glm::ivec2& size, Buffer& buffer)
{
    std::vector<float> pixels(size.x * size.y);
    buffer.CopyTo(pixels);

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

void CheckDiagonal(const glm::ivec2& size, Buffer& buffer, FluidSim& sim, float error = 1e-6)
{
    std::vector<float> pixels(size.x * size.y);
    buffer.CopyTo(pixels);

    for (std::size_t i = 0; i < size.x; i++)
    {
        for (std::size_t j = 0; j < size.y; j++)
        {
            std::size_t index = i + size.x * j;
            EXPECT_NEAR(sim.matrix(index, index), pixels[index], error);
        }
    }
}

void CheckWeights(const glm::ivec2& size, Buffer& buffer, FluidSim& sim, float error = 1e-6)
{
    std::vector<glm::vec2> pixels(size.x * size.y);
    buffer.CopyTo(pixels);

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

void CheckDiv(const glm::ivec2& size, Buffer& buffer, FluidSim& sim, float error = 1e-6)
{
    std::vector<float> pixels(size.x * size.y);
    buffer.CopyTo(pixels);

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

    NiceMock<LinearSolverMock> solver;

    Texture velocity(*device, size.x, size.y, vk::Format::eR32G32Sfloat, false);
    Texture solidPhi(*device, size.x, size.y, vk::Format::eR32Sfloat, false);
    Texture liquidPhi(*device, size.x, size.y, vk::Format::eR32Sfloat, false);
    Texture solidVelocity(*device, size.x, size.y, vk::Format::eR32G32Sfloat, false);

    BuildInputs(*device, size, sim, velocity, solidPhi, liquidPhi);

    Buffer* diagonalResult = nullptr;
    Buffer* lowerResult = nullptr;
    Buffer* divResult = nullptr;
    EXPECT_CALL(solver, Init(_, _, _, _))
            .WillOnce(Invoke([&](Buffer& d, Buffer& l, Buffer& b, Buffer& pressure)
            {
                diagonalResult = &d;
                lowerResult = &l;
                divResult = &b;
            }));

    Buffer valid(*device, vk::BufferUsageFlagBits::eStorageBuffer, true, size.x*size.y*sizeof(glm::ivec2));

    Pressure pressure(*device, 0.01f, size, solver, velocity, solidPhi, liquidPhi, solidVelocity, valid);

    LinearSolver::Parameters params(0);
    pressure.Solve(params);

    ASSERT_TRUE(diagonalResult != nullptr);
    ASSERT_TRUE(lowerResult != nullptr);
    ASSERT_TRUE(divResult != nullptr);

    Buffer diagonalOutput(*device, vk::BufferUsageFlagBits::eStorageBuffer, true, size.x*size.y*sizeof(float));
    Buffer lowerOutput(*device, vk::BufferUsageFlagBits::eStorageBuffer, true, size.x*size.y*sizeof(glm::vec2));
    Buffer divOutput(*device, vk::BufferUsageFlagBits::eStorageBuffer, true, size.x*size.y*sizeof(float));

    ExecuteCommand(*device, [&](vk::CommandBuffer commandBuffer)
    {
        diagonalOutput.CopyFrom(commandBuffer, *diagonalResult);
        lowerOutput.CopyFrom(commandBuffer, *lowerResult);
        divOutput.CopyFrom(commandBuffer, *divResult);
    });

    CheckDiagonal(size, diagonalOutput, sim, 1e-3f); // FIXME can we reduce error tolerance?
    CheckWeights(size, lowerOutput, sim, 1e-3f); // FIXME can we reduce error tolerance?
    CheckDiv(size, divOutput, sim);
}

TEST(PressureTest, LinearEquationSetup_Complex)
{
    glm::ivec2 size(50);

    FluidSim sim;
    sim.initialize(1.0f, size.x, size.y);
    sim.set_boundary(boundary_phi);

    AddParticles(size, sim, complex_boundary_phi);

    sim.add_force(0.01f);

    NiceMock<LinearSolverMock> solver;

    Texture velocity(*device, size.x, size.y, vk::Format::eR32G32Sfloat, false);
    Texture solidPhi(*device, size.x, size.y, vk::Format::eR32Sfloat, false);
    Texture liquidPhi(*device, size.x, size.y, vk::Format::eR32Sfloat, false);
    Texture solidVelocity(*device, size.x, size.y, vk::Format::eR32G32Sfloat, false);

    BuildInputs(*device, size, sim, velocity, solidPhi, liquidPhi);

    Buffer* diagonalResult = nullptr;
    Buffer* lowerResult = nullptr;
    Buffer* divResult = nullptr;
    EXPECT_CALL(solver, Init(_, _, _, _))
            .WillOnce(Invoke([&](Buffer& d, Buffer& l, Buffer& b, Buffer& pressure)
            {
                diagonalResult = &d;
                lowerResult = &l;
                divResult = &b;
            }));

    Buffer valid(*device, vk::BufferUsageFlagBits::eStorageBuffer, true, size.x*size.y*sizeof(glm::ivec2));

    Pressure pressure(*device, 0.01f, size, solver, velocity, solidPhi, liquidPhi, solidVelocity, valid);

    LinearSolver::Parameters params(0);
    pressure.Solve(params);

    ASSERT_TRUE(diagonalResult != nullptr);
    ASSERT_TRUE(lowerResult != nullptr);
    ASSERT_TRUE(divResult != nullptr);

    Buffer diagonalOutput(*device, vk::BufferUsageFlagBits::eStorageBuffer, true, size.x*size.y*sizeof(float));
    Buffer lowerOutput(*device, vk::BufferUsageFlagBits::eStorageBuffer, true, size.x*size.y*sizeof(glm::vec2));
    Buffer divOutput(*device, vk::BufferUsageFlagBits::eStorageBuffer, true, size.x*size.y*sizeof(float));

    ExecuteCommand(*device, [&](vk::CommandBuffer commandBuffer)
    {
        diagonalOutput.CopyFrom(commandBuffer, *diagonalResult);
        lowerOutput.CopyFrom(commandBuffer, *lowerResult);
        divOutput.CopyFrom(commandBuffer, *divResult);
    });

    CheckDiagonal(size, diagonalOutput, sim, 1e-3f); // FIXME can we reduce error tolerance?
    CheckWeights(size, lowerOutput, sim, 1e-3f); // FIXME can we reduce error tolerance?
    CheckDiv(size, divOutput, sim);
}

TEST(PressureTest, Project_Simple)
{
    glm::ivec2 size(50);

    FluidSim sim;
    sim.initialize(1.0f, size.x, size.y);
    sim.set_boundary(boundary_phi);

    AddParticles(size, sim, boundary_phi);

    sim.add_force(0.01f);

    NiceMock<LinearSolverMock> solver;

    Texture velocity(*device, size.x, size.y, vk::Format::eR32G32Sfloat, false);
    Texture solidPhi(*device, size.x, size.y, vk::Format::eR32Sfloat, false);
    Texture liquidPhi(*device, size.x, size.y, vk::Format::eR32Sfloat, false);
    Texture solidVelocity(*device, size.x, size.y, vk::Format::eR32G32Sfloat, false);

    BuildInputs(*device, size, sim, velocity, solidPhi, liquidPhi);

    Buffer computedPressure(*device, vk::BufferUsageFlagBits::eStorageBuffer, true, size.x*size.y*sizeof(float));
    std::vector<float> computedPressureData(size.x*size.y, 0.0f);
    for (std::size_t i = 0; i < computedPressureData.size(); i++)
    {
        computedPressureData[i] = (float)sim.pressure[i];
    }
    computedPressure.CopyFrom(computedPressureData);

    EXPECT_CALL(solver, Init(_, _, _, _))
        .WillOnce(Invoke([&](Buffer& d, Buffer& l, Buffer& div, Buffer& pressure)
        {
            ExecuteCommand(*device, [&](vk::CommandBuffer commandBuffer)
            {
                pressure.CopyFrom(commandBuffer, computedPressure);
            });
        }));

    Buffer valid(*device, vk::BufferUsageFlagBits::eStorageBuffer, true, size.x*size.y*sizeof(glm::ivec2));

    Pressure pressure(*device, 0.01f, size, solver, velocity, solidPhi, liquidPhi, solidVelocity, valid);

    LinearSolver::Parameters params(0);
    pressure.Solve(params);

    Texture outputVelocity(*device, size.x, size.y, vk::Format::eR32G32Sfloat, true);

    ExecuteCommand(*device, [&](vk::CommandBuffer commandBuffer)
    {
        outputVelocity.CopyFrom(commandBuffer, velocity);
    });

    CheckVelocity(size, outputVelocity, sim);
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

    NiceMock<LinearSolverMock> solver;

    Texture velocity(*device, size.x, size.y, vk::Format::eR32G32Sfloat, false);
    Texture solidPhi(*device, size.x, size.y, vk::Format::eR32Sfloat, false);
    Texture liquidPhi(*device, size.x, size.y, vk::Format::eR32Sfloat, false);
    Texture solidVelocity(*device, size.x, size.y, vk::Format::eR32G32Sfloat, false);

    BuildInputs(*device, size, sim, velocity, solidPhi, liquidPhi);

    Buffer computedPressure(*device, vk::BufferUsageFlagBits::eStorageBuffer, true, size.x*size.y*sizeof(float));
    std::vector<float> computedPressureData(size.x*size.y, 0.0f);
    for (std::size_t i = 0; i < computedPressureData.size(); i++)
    {
        computedPressureData[i] = (float)sim.pressure[i];
    }
    computedPressure.CopyFrom(computedPressureData);

    EXPECT_CALL(solver, Init(_, _, _, _))
        .WillOnce(Invoke([&](Buffer& d, Buffer& l, Buffer& b, Buffer& pressure)
        {
            ExecuteCommand(*device, [&](vk::CommandBuffer commandBuffer)
            {
                pressure.CopyFrom(commandBuffer, computedPressure);
            });
        }));

    Buffer valid(*device, vk::BufferUsageFlagBits::eStorageBuffer, true, size.x*size.y*sizeof(glm::ivec2));

    Pressure pressure(*device, 0.01f, size, solver, velocity, solidPhi, liquidPhi, solidVelocity, valid);

    LinearSolver::Parameters params(0);
    pressure.Solve(params);

    Texture outputVelocity(*device, size.x, size.y, vk::Format::eR32G32Sfloat, true);

    ExecuteCommand(*device, [&](vk::CommandBuffer commandBuffer)
    {
        outputVelocity.CopyFrom(commandBuffer, velocity);
    });

    CheckVelocity(size, outputVelocity, sim);
    CheckValid(size, sim, valid);
}
