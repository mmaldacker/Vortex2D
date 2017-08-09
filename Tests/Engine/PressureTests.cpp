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
    MOCK_METHOD3(Init, void(Buffer& maxtrix, Buffer& b, Buffer& pressure));
    MOCK_METHOD1(Solve, void(Parameters& params));
};

void PrintDiv(const glm::vec2& size, Buffer& buffer)
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

void PrintDiagonal(const glm::vec2& size, Buffer& buffer)
{
    std::vector<LinearSolver::Data> pixels(size.x * size.y);
    buffer.CopyTo(pixels);

    for (std::size_t j = 0; j < size.y; j++)
    {
        for (std::size_t i = 0; i < size.x; i++)
        {
            std::size_t index = i + size.x * j;
            std::cout << "(" <<  pixels[index].Diagonal << ")";
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;
}

void PrintWeights(const glm::vec2& size, FluidSim& sim)
{
    for (std::size_t j = 1; j < size.y - 1; j++)
    {
        for (std::size_t i = 1; i < size.x - 1; i++)
        {
            std::size_t index = i + size.x * j;
            std::cout << "(" <<  sim.matrix(index + 1, index) << ","
                      << sim.matrix(index - 1, index) << ","
                      << sim.matrix(index, index + size.x) << ","
                      << sim.matrix(index, index - size.x) << ")";
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;
}

void PrintDiagonal(const glm::vec2& size, FluidSim& sim)
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

void CheckDiagonal(const glm::vec2& size, Buffer& buffer, FluidSim& sim, float error = 1e-6)
{
    std::vector<LinearSolver::Data> pixels(size.x * size.y);
    buffer.CopyTo(pixels);

    for (std::size_t i = 0; i < size.x; i++)
    {
        for (std::size_t j = 0; j < size.y; j++)
        {
            std::size_t index = i + size.x * j;
            EXPECT_NEAR(sim.matrix(index, index), pixels[index].Diagonal, error);
        }
    }
}

void CheckWeights(const glm::vec2& size, Buffer& buffer, FluidSim& sim, float error = 1e-6)
{
    std::vector<LinearSolver::Data> pixels(size.x * size.y);
    buffer.CopyTo(pixels);

    for (std::size_t i = 1; i < size.x - 1; i++)
    {
        for (std::size_t j = 1; j < size.y - 1; j++)
        {
            std::size_t index = i + size.x * j;
            EXPECT_NEAR(sim.matrix(index + 1, index), pixels[index].Weights.x, error);
            EXPECT_NEAR(sim.matrix(index - 1, index), pixels[index].Weights.y, error);
            EXPECT_NEAR(sim.matrix(index, index + size.x), pixels[index].Weights.z, error);
            EXPECT_NEAR(sim.matrix(index, index - size.x), pixels[index].Weights.w, error);
        }
    }
}

void CheckDiv(const glm::vec2& size, Buffer& buffer, FluidSim& sim, float error = 1e-6)
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

void CheckVelocity(const glm::vec2& size, Vortex2D::Renderer::Texture& buffer, FluidSim& sim, float error = 1e-6)
{
    std::vector<glm::vec2> pixels(size.x * size.y);
    buffer.CopyTo(pixels);

    // FIXME need to check the entire velocity buffer
    for (std::size_t i = 1; i < size.x; i++)
    {
        for (std::size_t j = 1; j < size.y; j++)
        {
            auto uv = pixels[i + j * size.x];
            EXPECT_NEAR(sim.u(i, j), uv.x, error) << "Mismatch at " << i << "," << j;
            EXPECT_NEAR(sim.v(i, j), uv.y, error) << "Mismatch at " << i << "," << j;
        }
    }
}

void PrintVelocity(const glm::vec2& size, Texture& buffer)
{
    std::vector<glm::vec2> pixels(size.x * size.y);
    buffer.CopyTo(pixels);

    for (std::size_t j = 0; j < size.y; j++)
    {
        for (std::size_t i = 0; i < size.x; i++)
        {
            std::size_t index = i + size.x * j;
            std::cout << "(" <<  pixels[index].x << "," << pixels[index].y << ")";
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;
}

void PrintVelocity(const glm::vec2& size, FluidSim& sim)
{
    for (std::size_t j = 0; j < size.y; j++)
    {
        for (std::size_t i = 0; i < size.x; i++)
        {
            std::cout << "(" << sim.u(i, j) << "," << sim.v(i, j) << ")";
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;
}

void BuildInputs(const glm::vec2& size, FluidSim& sim, Texture& velocity, Texture& solidPhi, Texture& liquidPhi)
{
    Texture inputVelocity(*device, size.x, size.y, vk::Format::eR32G32Sfloat, true);
    SetVelocity(size, inputVelocity, sim);

    sim.project(0.01f);

    Texture inputSolidPhi(*device, size.x, size.y, vk::Format::eR32Sfloat, true);
    SetSolidPhi(size, inputSolidPhi, sim);

    Texture inputLiquidPhi(*device, size.x, size.y, vk::Format::eR32Sfloat, true);
    SetLiquidPhi(size, inputLiquidPhi, sim);

    ExecuteCommand(*device, [&](vk::CommandBuffer commandBuffer)
    {
       velocity.CopyFrom(commandBuffer, inputVelocity);
       solidPhi.CopyFrom(commandBuffer, inputSolidPhi);
       liquidPhi.CopyFrom(commandBuffer, inputLiquidPhi);
    });
}

TEST(PressureTest, LinearEquationSetup_Simple)
{
    glm::vec2 size(50);

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

    BuildInputs(size, sim, velocity, solidPhi, liquidPhi);

    Buffer* matrixResult = nullptr;
    Buffer* divResult = nullptr;
    EXPECT_CALL(solver, Init(_, _, _))
            .WillOnce(Invoke([&](Buffer& matrix, Buffer& b, Buffer& pressure)
            {
                matrixResult = &matrix;
                divResult = &b;
            }));

    Pressure pressure(*device, 0.01f, size, solver, velocity, solidPhi, liquidPhi, solidVelocity);

    LinearSolver::Parameters params(0);
    pressure.Solve(params);

    ASSERT_TRUE(matrixResult != nullptr);
    ASSERT_TRUE(divResult != nullptr);

    Buffer matrixOutput(*device, vk::BufferUsageFlagBits::eStorageBuffer, true, size.x*size.y*sizeof(LinearSolver::Data));
    Buffer divOutput(*device, vk::BufferUsageFlagBits::eStorageBuffer, true, size.x*size.y*sizeof(float));

    ExecuteCommand(*device, [&](vk::CommandBuffer commandBuffer)
    {
        matrixOutput.CopyFrom(commandBuffer, *matrixResult);
        divOutput.CopyFrom(commandBuffer, *divResult);
    });

    CheckDiagonal(size, matrixOutput, sim, 1e-3); // FIXME can we reduce error tolerance?
    CheckWeights(size, matrixOutput, sim, 1e-3); // FIXME can we reduce error tolerance?
    CheckDiv(size, divOutput, sim);
}

TEST(PressureTest, LinearEquationSetup_Complex)
{
    glm::vec2 size(50);

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

    BuildInputs(size, sim, velocity, solidPhi, liquidPhi);

    Buffer* matrixResult = nullptr;
    Buffer* divResult = nullptr;
    EXPECT_CALL(solver, Init(_, _, _))
            .WillOnce(Invoke([&](Buffer& matrix, Buffer& b, Buffer& pressure)
            {
                matrixResult = &matrix;
                divResult = &b;
            }));

    Pressure pressure(*device, 0.01f, size, solver, velocity, solidPhi, liquidPhi, solidVelocity);

    LinearSolver::Parameters params(0);
    pressure.Solve(params);

    ASSERT_TRUE(matrixResult != nullptr);
    ASSERT_TRUE(divResult != nullptr);

    Buffer matrixOutput(*device, vk::BufferUsageFlagBits::eStorageBuffer, true, size.x*size.y*sizeof(LinearSolver::Data));
    Buffer divOutput(*device, vk::BufferUsageFlagBits::eStorageBuffer, true, size.x*size.y*sizeof(float));

    ExecuteCommand(*device, [&](vk::CommandBuffer commandBuffer)
    {
        matrixOutput.CopyFrom(commandBuffer, *matrixResult);
        divOutput.CopyFrom(commandBuffer, *divResult);
    });

    CheckDiagonal(size, matrixOutput, sim, 1e-3); // FIXME can we reduce error tolerance?
    CheckWeights(size, matrixOutput, sim, 1e-3); // FIXME can we reduce error tolerance?
    CheckDiv(size, divOutput, sim);
}

TEST(PressureTest, Project_Simple)
{
    glm::vec2 size(50);

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

    BuildInputs(size, sim, velocity, solidPhi, liquidPhi);

    Buffer computedPressure(*device, vk::BufferUsageFlagBits::eStorageBuffer, true, size.x*size.y*sizeof(float));
    std::vector<float> computedPressureData(size.x*size.y, 0.0f);
    for (std::size_t i = 0; i < computedPressureData.size(); i++)
    {
        computedPressureData[i] = sim.pressure[i];
    }
    computedPressure.CopyFrom(computedPressureData);

    EXPECT_CALL(solver, Init(_, _, _))
        .WillOnce(Invoke([&](Buffer& data, Buffer& div, Buffer& pressure)
        {
            ExecuteCommand(*device, [&](vk::CommandBuffer commandBuffer)
            {
                pressure.CopyFrom(commandBuffer, computedPressure);
            });
        }));


    Pressure pressure(*device, 0.01f, size, solver, velocity, solidPhi, liquidPhi, solidVelocity);

    LinearSolver::Parameters params(0);
    pressure.Solve(params);

    Texture outputVelocity(*device, size.x, size.y, vk::Format::eR32G32Sfloat, true);

    ExecuteCommand(*device, [&](vk::CommandBuffer commandBuffer)
    {
        outputVelocity.CopyFrom(commandBuffer, velocity);
    });

    CheckVelocity(size, outputVelocity, sim);
}

TEST(PressureTest, Project_Complex)
{
    glm::vec2 size(50);

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

    BuildInputs(size, sim, velocity, solidPhi, liquidPhi);

    Buffer computedPressure(*device, vk::BufferUsageFlagBits::eStorageBuffer, true, size.x*size.y*sizeof(float));
    std::vector<float> computedPressureData(size.x*size.y, 0.0f);
    for (std::size_t i = 0; i < computedPressureData.size(); i++)
    {
        computedPressureData[i] = sim.pressure[i];
    }
    computedPressure.CopyFrom(computedPressureData);

    EXPECT_CALL(solver, Init(_, _, _))
        .WillOnce(Invoke([&](Buffer& data, Buffer& b, Buffer& pressure)
        {
            ExecuteCommand(*device, [&](vk::CommandBuffer commandBuffer)
            {
                pressure.CopyFrom(commandBuffer, computedPressure);
            });
        }));

    Pressure pressure(*device, 0.01f, size, solver, velocity, solidPhi, liquidPhi, solidVelocity);

    LinearSolver::Parameters params(0);
    pressure.Solve(params);

    Texture outputVelocity(*device, size.x, size.y, vk::Format::eR32G32Sfloat, true);

    ExecuteCommand(*device, [&](vk::CommandBuffer commandBuffer)
    {
        outputVelocity.CopyFrom(commandBuffer, velocity);
    });
}
