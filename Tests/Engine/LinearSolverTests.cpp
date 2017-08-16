//
//  LinearSolverTests.cpp
//  Vortex2D
//

#include "Verify.h"
#include "VariationalHelpers.h"
#include <Vortex2D/Engine/LinearSolver/Reduce.h>
#include <Vortex2D/Engine/LinearSolver/GaussSeidel.h>
#include <Vortex2D/Engine/LinearSolver/ConjugateGradient.h>
#include <Vortex2D/Engine/LinearSolver/Diagonal.h>
#include <Vortex2D/Engine/LinearSolver/IncompletePoisson.h>
#include <Vortex2D/Engine/Pressure.h>

#include <algorithm>
#include <chrono>
#include <numeric>

using namespace Vortex2D::Renderer;
using namespace Vortex2D::Fluid;

extern Device* device;

TEST(LinearSolverTests, ReduceSum)
{
    glm::ivec2 size(10, 15);
    int n = size.x * size.y;
    Buffer input(*device, vk::BufferUsageFlagBits::eStorageBuffer, true, sizeof(float) * n);
    Buffer output(*device, vk::BufferUsageFlagBits::eStorageBuffer, true, sizeof(float));

    ReduceSum reduce(*device, size);
    auto reduceBound = reduce.Bind(input, output);

    std::vector<float> inputData(n);

    {
        float n = 1.0f;
        std::generate(inputData.begin(), inputData.end(), [&n]{ return n++; });
    }

    input.CopyFrom(inputData);

    ExecuteCommand(*device, [&](vk::CommandBuffer commandBuffer)
    {
       reduceBound.Record(commandBuffer);
    });

    std::vector<float> outputData(1, 0.0f);
    output.CopyTo(outputData);

    ASSERT_EQ(0.5f * n * (n + 1), outputData[0]);
}

TEST(LinearSolverTests, ReduceBigSum)
{
    glm::ivec2 size(500, 500);
    int n = size.x * size.y; // 1 million

    Buffer input(*device, vk::BufferUsageFlagBits::eStorageBuffer, true, sizeof(float) * n);
    Buffer output(*device, vk::BufferUsageFlagBits::eStorageBuffer, true, sizeof(float));

    ReduceSum reduce(*device, size);
    auto reduceBound = reduce.Bind(input, output);

    std::vector<float> inputData(n, 1.0f);

    {
        float n = 1.0f;
        std::generate(inputData.begin(), inputData.end(), [&n]{ return n++; });
    }

    input.CopyFrom(inputData);

    ExecuteCommand(*device, [&](vk::CommandBuffer commandBuffer)
    {
       reduceBound.Record(commandBuffer);
    });

    std::vector<float> outputData(1, 0.0f);
    output.CopyTo(outputData);

    ASSERT_EQ(0.5f * n * (n + 1), outputData[0]);
}

TEST(LinearSolverTests, ReduceMax)
{
    glm::ivec2 size(10, 15);
    int n = size.x * size.y;

    Buffer input(*device, vk::BufferUsageFlagBits::eStorageBuffer, true, sizeof(float) * n);
    Buffer output(*device, vk::BufferUsageFlagBits::eStorageBuffer, true, sizeof(float));

    ReduceMax reduce(*device, size);
    auto reduceBound = reduce.Bind(input, output);

    std::vector<float> inputData(10*15);

    {
        float n = -1.0f;
        std::generate(inputData.begin(), inputData.end(), [&n]{ return n--; });
    }

    input.CopyFrom(inputData);

    ExecuteCommand(*device, [&](vk::CommandBuffer commandBuffer)
    {
       reduceBound.Record(commandBuffer);
    });

    std::vector<float> outputData(1, 0.0f);
    output.CopyTo(outputData);

    ASSERT_EQ(150.0f, outputData[0]);
}

void CheckPressure(const glm::ivec2& size, const std::vector<double>& pressure, Buffer& bufferPressure, float error)
{
    std::vector<float> bufferPressureData(size.x * size.y);
    bufferPressure.CopyTo(bufferPressureData);

    for (int i = 0; i < size.x; i++)
    {
        for (int j = 0; j < size.y; j++)
        {
            std::size_t index = i + j * size.x;
            float value = (float)pressure[index];
            EXPECT_NEAR(value, bufferPressureData[index], error) << "Mismatch at " << i << ", " << j << "\n";
        }
    }
}

TEST(LinearSolverTests, Simple_SOR)
{
    glm::ivec2 size(50);

    FluidSim sim;
    sim.initialize(1.0f, size.x, size.y);
    sim.set_boundary(boundary_phi);

    AddParticles(size, sim, boundary_phi);

    sim.add_force(0.01f);
    sim.project(0.01f);

    Buffer matrix(*device, vk::BufferUsageFlagBits::eStorageBuffer, true, size.x*size.y*sizeof(LinearSolver::Data));
    Buffer div(*device, vk::BufferUsageFlagBits::eStorageBuffer, true, size.x*size.y*sizeof(float));
    Buffer pressure(*device, vk::BufferUsageFlagBits::eStorageBuffer, true, size.x*size.y*sizeof(float));

    BuildLinearEquation(size, matrix, div, sim);

    LinearSolver::Parameters params(1000, 1e-4f);
    GaussSeidel solver(*device, size);

    solver.Init(matrix, div, pressure);
    solver.Solve(params);

    device->Queue().waitIdle();

    CheckPressure(size, sim.pressure, pressure, 1e-4f);

    std::cout << "Solved with number of iterations: " << params.OutIterations << std::endl;
}

TEST(LinearSolverTests, Complex_SOR)
{
    glm::ivec2 size(50);

    FluidSim sim;
    sim.initialize(1.0f, size.x, size.y);
    sim.set_boundary(complex_boundary_phi);

    AddParticles(size, sim, complex_boundary_phi);

    sim.add_force(0.01f);
    sim.project(0.01f);

    Buffer matrix(*device, vk::BufferUsageFlagBits::eStorageBuffer, true, size.x*size.y*sizeof(LinearSolver::Data));
    Buffer div(*device, vk::BufferUsageFlagBits::eStorageBuffer, true, size.x*size.y*sizeof(float));
    Buffer pressure(*device, vk::BufferUsageFlagBits::eStorageBuffer, true, size.x*size.y*sizeof(float));

    BuildLinearEquation(size, matrix, div, sim);

    LinearSolver::Parameters params(1000, 1e-4f);
    GaussSeidel solver(*device, size);

    solver.Init(matrix, div, pressure);
    solver.Solve(params);

    device->Queue().waitIdle();

    CheckPressure(size, sim.pressure, pressure, 1e-4f);

    std::cout << "Solved with number of iterations: " << params.OutIterations << std::endl;
}

TEST(LinearSolverTests, Simple_CG)
{
    glm::ivec2 size(50);

    FluidSim sim;
    sim.initialize(1.0f, size.x, size.y);
    sim.set_boundary(boundary_phi);

    AddParticles(size, sim, boundary_phi);

    sim.add_force(0.01f);
    sim.project(0.01f);

    Buffer matrix(*device, vk::BufferUsageFlagBits::eStorageBuffer, true, size.x*size.y*sizeof(LinearSolver::Data));
    Buffer div(*device, vk::BufferUsageFlagBits::eStorageBuffer, true, size.x*size.y*sizeof(float));
    Buffer pressure(*device, vk::BufferUsageFlagBits::eStorageBuffer, true, size.x*size.y*sizeof(float));

    BuildLinearEquation(size, matrix, div, sim);

    Diagonal preconditioner(*device, size);

    LinearSolver::Parameters params(1000, 1e-5f);
    ConjugateGradient solver(*device, size, preconditioner);

    solver.Init(matrix, div, pressure);
    solver.NormalSolve(params);

    device->Queue().waitIdle();

    CheckPressure(size, sim.pressure, pressure, 1e-4f); // TODO somehow error is bigger than 1e-5

    std::cout << "Solved with number of iterations: " << params.OutIterations << std::endl;
}

TEST(LinearSolverTests, Diagonal_Simple_PCG)
{
    glm::ivec2 size(50);

    FluidSim sim;
    sim.initialize(1.0f, size.x, size.y);
    sim.set_boundary(boundary_phi);

    AddParticles(size, sim, boundary_phi);

    sim.add_force(0.01f);
    sim.project(0.01f);

    Buffer matrix(*device, vk::BufferUsageFlagBits::eStorageBuffer, true, size.x*size.y*sizeof(LinearSolver::Data));
    Buffer div(*device, vk::BufferUsageFlagBits::eStorageBuffer, true, size.x*size.y*sizeof(float));
    Buffer pressure(*device, vk::BufferUsageFlagBits::eStorageBuffer, true, size.x*size.y*sizeof(float));

    BuildLinearEquation(size, matrix, div, sim);

    Diagonal preconditioner(*device, size);

    LinearSolver::Parameters params(1000, 1e-5f);
    ConjugateGradient solver(*device, size, preconditioner);

    solver.Init(matrix, div, pressure);
    solver.Solve(params);

    device->Queue().waitIdle();

    CheckPressure(size, sim.pressure, pressure, 1e-5f);

    std::cout << "Solved with number of iterations: " << params.OutIterations << std::endl;
}

TEST(LinearSolverTests, GaussSeidel_Simple_PCG)
{
    glm::ivec2 size(50);

    FluidSim sim;
    sim.initialize(1.0f, size.x, size.y);
    sim.set_boundary(boundary_phi);

    AddParticles(size, sim, boundary_phi);

    sim.add_force(0.01f);
    sim.project(0.01f);

    Buffer matrix(*device, vk::BufferUsageFlagBits::eStorageBuffer, true, size.x*size.y*sizeof(LinearSolver::Data));
    Buffer div(*device, vk::BufferUsageFlagBits::eStorageBuffer, true, size.x*size.y*sizeof(float));
    Buffer pressure(*device, vk::BufferUsageFlagBits::eStorageBuffer, true, size.x*size.y*sizeof(float));

    BuildLinearEquation(size, matrix, div, sim);

    GaussSeidel preconditioner(*device, size);
    preconditioner.SetW(1.0f);

    LinearSolver::Parameters params(1000, 1e-5f);
    ConjugateGradient solver(*device, size, preconditioner);

    solver.Init(matrix, div, pressure);
    solver.Solve(params);

    device->Queue().waitIdle();

    CheckPressure(size, sim.pressure, pressure, 1e-4f);

    std::cout << "Solved with number of iterations: " << params.OutIterations << std::endl;
}

TEST(LinearSolverTests, IncompletePoisson_Simple_PCG)
{
    glm::ivec2 size(500);

    FluidSim sim;
    sim.initialize(1.0f, size.x, size.y);
    sim.set_boundary(boundary_phi);

    AddParticles(size, sim, boundary_phi);

    sim.add_force(0.01f);
    sim.project(0.01f);

    Buffer matrix(*device, vk::BufferUsageFlagBits::eStorageBuffer, true, size.x*size.y*sizeof(LinearSolver::Data));
    Buffer div(*device, vk::BufferUsageFlagBits::eStorageBuffer, true, size.x*size.y*sizeof(float));
    Buffer pressure(*device, vk::BufferUsageFlagBits::eStorageBuffer, true, size.x*size.y*sizeof(float));

    BuildLinearEquation(size, matrix, div, sim);

    IncompletePoisson preconditioner(*device, size);

    LinearSolver::Parameters params(1000, 1e-5f);
    ConjugateGradient solver(*device, size, preconditioner);

    solver.Init(matrix, div, pressure);
    solver.Solve(params);

    device->Queue().waitIdle();

    CheckPressure(size, sim.pressure, pressure, 1e-5f);

    std::cout << "Solved with number of iterations: " << params.OutIterations << std::endl;
}

TEST(LinearSolverTests, Zero_CG)
{
    glm::vec2 size(50);

    Buffer matrix(*device, vk::BufferUsageFlagBits::eStorageBuffer, true, size.x*size.y*sizeof(LinearSolver::Data));
    Buffer div(*device, vk::BufferUsageFlagBits::eStorageBuffer, true, size.x*size.y*sizeof(float));
    Buffer pressure(*device, vk::BufferUsageFlagBits::eStorageBuffer, true, size.x*size.y*sizeof(float));

    Diagonal preconditioner(*device, size);

    LinearSolver::Parameters params(1000, 1e-5f);
    ConjugateGradient solver(*device, size, preconditioner);

    solver.Init(matrix, div, pressure);
    solver.NormalSolve(params);

    device->Queue().waitIdle();

    std::vector<float> data(size.x*size.y, 1.0f);
    pressure.CopyTo(data);

    for (int i = 0; i < size.x; i++)
    {
        for (int j = 0; j < size.y; j++)
        {
            EXPECT_FLOAT_EQ(0.0f, data[i + size.x * j]);
        }
    }
}

TEST(LinearSolverTests, Zero_PCG)
{
    glm::vec2 size(50);

    Buffer matrix(*device, vk::BufferUsageFlagBits::eStorageBuffer, true, size.x*size.y*sizeof(LinearSolver::Data));
    Buffer div(*device, vk::BufferUsageFlagBits::eStorageBuffer, true, size.x*size.y*sizeof(float));
    Buffer pressure(*device, vk::BufferUsageFlagBits::eStorageBuffer, true, size.x*size.y*sizeof(float));

    Diagonal preconditioner(*device, size);

    LinearSolver::Parameters params(1000, 1e-5f);
    ConjugateGradient solver(*device, size, preconditioner);

    solver.Init(matrix, div, pressure);
    solver.Solve(params);

    device->Queue().waitIdle();

    std::vector<float> data(size.x*size.y, 1.0f);
    pressure.CopyTo(data);

    for (int i = 0; i < size.x; i++)
    {
        for (int j = 0; j < size.y; j++)
        {
            EXPECT_FLOAT_EQ(0.0f, data[i + size.x * j]);
        }
    }
}
