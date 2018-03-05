//
//  LinearSolverTests.cpp
//  Vortex2D
//

#include "Verify.h"
#include "VariationalHelpers.h"
#include <Vortex2D/Engine/LinearSolver/Reduce.h>
#include <Vortex2D/Engine/LinearSolver/GaussSeidel.h>
#include <Vortex2D/Engine/LinearSolver/Transfer.h>
#include <Vortex2D/Engine/LinearSolver/Multigrid.h>
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
    int total_size = size.x * size.y;
    Buffer<float> input(*device, total_size, VMA_MEMORY_USAGE_CPU_ONLY);
    Buffer<float> output(*device, 1, VMA_MEMORY_USAGE_CPU_ONLY);

    ReduceSum reduce(*device, size);
    auto reduceBound = reduce.Bind(input, output);

    std::vector<float> inputData(total_size);

    {
        float n = 1.0f;
        std::generate(inputData.begin(), inputData.end(), [&n]{ return n++; });
    }

    CopyFrom(input, inputData);

    ExecuteCommand(*device, [&](vk::CommandBuffer commandBuffer)
    {
       reduceBound.Record(commandBuffer);
    });

    std::vector<float> outputData(1, 0.0f);
    CopyTo(output, outputData);

    ASSERT_EQ(0.5f * total_size * (total_size + 1), outputData[0]);
}

TEST(LinearSolverTests, ReduceBigSum)
{
    glm::ivec2 size(500);
    int total_size = size.x * size.y; // 1 million

    Buffer<float> input(*device, total_size, VMA_MEMORY_USAGE_CPU_ONLY);
    Buffer<float> output(*device, 1, VMA_MEMORY_USAGE_CPU_ONLY);

    ReduceSum reduce(*device, size);
    auto reduceBound = reduce.Bind(input, output);

    std::vector<float> inputData(total_size, 1.0f);

    {
        float n = 1.0f;
        std::generate(inputData.begin(), inputData.end(), [&n]{ return n++; });
    }

    CopyFrom(input, inputData);

    ExecuteCommand(*device, [&](vk::CommandBuffer commandBuffer)
    {
       reduceBound.Record(commandBuffer);
    });

    std::vector<float> outputData(1, 0.0f);
    CopyTo(output, outputData);

    ASSERT_EQ(0.5f * total_size * (total_size + 1), outputData[0]);
}

TEST(LinearSolverTests, ReduceMax)
{
    glm::ivec2 size(10, 15);
    int total_size = size.x * size.y;

    Buffer<float> input(*device, total_size, VMA_MEMORY_USAGE_CPU_ONLY);
    Buffer<float> output(*device, 1, VMA_MEMORY_USAGE_CPU_ONLY);

    ReduceMax reduce(*device, size);
    auto reduceBound = reduce.Bind(input, output);

    std::vector<float> inputData(10*15);

    {
        float n = -1.0f;
        std::generate(inputData.begin(), inputData.end(), [&n]{ return n--; });
    }

    CopyFrom(input, inputData);

    ExecuteCommand(*device, [&](vk::CommandBuffer commandBuffer)
    {
       reduceBound.Record(commandBuffer);
    });

    std::vector<float> outputData(1, 0.0f);
    CopyTo(output, outputData);

    ASSERT_EQ(150.0f, outputData[0]);
}

TEST(LinearSolverTests, Transfer_Prolongate)
{
    glm::ivec2 coarseSize(2);
    glm::ivec2 fineSize(4);

    Transfer t(*device);

    Buffer<float> fineDiagonal(*device, fineSize.x*fineSize.y, VMA_MEMORY_USAGE_CPU_ONLY);
    std::vector<float> fineDiagonalData(fineSize.x*fineSize.y, {1.0f});
    CopyFrom(fineDiagonal, fineDiagonalData);

    Buffer<float> coarseDiagonal(*device, coarseSize.x*coarseSize.y, VMA_MEMORY_USAGE_CPU_ONLY);
    std::vector<float> coarseDiagonalData(coarseSize.x*coarseSize.y, {1.0f});
    CopyFrom(coarseDiagonal, coarseDiagonalData);

    Buffer<float> input(*device, coarseSize.x*coarseSize.y, VMA_MEMORY_USAGE_CPU_ONLY);
    Buffer<float> output(*device, fineSize.x*fineSize.y, VMA_MEMORY_USAGE_CPU_ONLY);

    std::vector<float> data(coarseSize.x * coarseSize.y, 0.0f);
    std::iota(data.begin(), data.end(), 1.0f);
    CopyFrom(input, data);

    t.ProlongateBind(0, fineSize, output, fineDiagonal, input, coarseDiagonal);
    ExecuteCommand(*device, [&](vk::CommandBuffer commandBuffer)
    {
        t.Prolongate(commandBuffer, 0);
    });

    std::vector<float> outputData(fineSize.x*fineSize.y, 0.0f);
    CopyTo(output, outputData);

    EXPECT_FLOAT_EQ(1.0f, outputData[1 + fineSize.x * 1]);
    EXPECT_FLOAT_EQ(2.0f, outputData[2 + fineSize.x * 1]);
    EXPECT_FLOAT_EQ(3.0f, outputData[1 + fineSize.x * 2]);
    EXPECT_FLOAT_EQ(4.0f, outputData[2 + fineSize.x * 2]);
}

TEST(LinearSolverTests, Transfer_Restrict)
{
    glm::ivec2 coarseSize(2);
    glm::ivec2 fineSize(4);

    Transfer t(*device);

    Buffer<float> fineDiagonal(*device, fineSize.x*fineSize.y, VMA_MEMORY_USAGE_CPU_ONLY);
    std::vector<float> fineDiagonalData(fineSize.x*fineSize.y, {1.0f});
    CopyFrom(fineDiagonal, fineDiagonalData);

    Buffer<float> coarseDiagonal(*device, coarseSize.x*coarseSize.y, VMA_MEMORY_USAGE_CPU_ONLY);
    std::vector<float> coarseDiagonalData(coarseSize.x*coarseSize.y, {1.0f});
    CopyFrom(coarseDiagonal, coarseDiagonalData);

    Buffer<float> input(*device, fineSize.x*fineSize.y, VMA_MEMORY_USAGE_CPU_ONLY);
    Buffer<float> output(*device, coarseSize.x*coarseSize.y, VMA_MEMORY_USAGE_CPU_ONLY);

    std::vector<float> data(fineSize.x * fineSize.y, 1.0f);
    std::iota(data.begin(), data.end(), 1.0f);
    CopyFrom(input, data);

    t.RestrictBind(0, fineSize, input, fineDiagonal, output, coarseDiagonal);
    ExecuteCommand(*device, [&](vk::CommandBuffer commandBuffer)
    {
        t.Restrict(commandBuffer, 0);
    });

    std::vector<float> outputData(coarseSize.x*coarseSize.y, 1.0f);
    CopyTo(output, outputData);

    EXPECT_FLOAT_EQ((1.0f + 2.0f + 5.0f + 6.0f) / 4.0f, outputData[0 + coarseSize.x * 0]);
    EXPECT_FLOAT_EQ((3.0f + 4.0f + 7.0f + 8.0f) / 4.0f, outputData[1 + coarseSize.x * 0]);
    EXPECT_FLOAT_EQ((9.0f + 10.0f + 13.0f + 14.0f) / 4.0f, outputData[0 + coarseSize.x * 1]);
    EXPECT_FLOAT_EQ((11.0f + 12.0f + 15.0f + 16.0f) / 4.0f, outputData[1 + coarseSize.x * 1]);
}

void CheckPressure(const glm::ivec2& size, const std::vector<double>& pressure, Buffer<float>& bufferPressure, float error)
{
    std::vector<float> bufferPressureData(size.x * size.y);
    CopyTo(bufferPressure, bufferPressureData);

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
    sim.compute_phi();
    sim.extrapolate_phi();
    sim.apply_projection(0.01f);

    LinearSolver::Data data(*device, size, VMA_MEMORY_USAGE_CPU_ONLY);

    BuildLinearEquation(size, data.Diagonal, data.Lower, data.B, sim);

    LinearSolver::Parameters params(1000, 1e-4f);
    GaussSeidel solver(*device, size);

    solver.Bind(data.Diagonal, data.Lower, data.B, data.X);
    solver.Solve(params);

    device->Queue().waitIdle();

    CheckPressure(size, sim.pressure, data.X, 1e-4f);

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
    sim.compute_phi();
    sim.extrapolate_phi();
    sim.apply_projection(0.01f);

    LinearSolver::Data data(*device, size, VMA_MEMORY_USAGE_CPU_ONLY);

    BuildLinearEquation(size, data.Diagonal, data.Lower, data.B, sim);

    LinearSolver::Parameters params(1000, 1e-4f);
    GaussSeidel solver(*device, size);

    solver.Bind(data.Diagonal, data.Lower, data.B, data.X);
    solver.Solve(params);

    device->Queue().waitIdle();

    CheckPressure(size, sim.pressure, data.X, 1e-4f);

    std::cout << "Solved with number of iterations: " << params.OutIterations << std::endl;
}

TEST(LinearSolverTests, LocalGaussSeidel)
{
    glm::ivec2 size(16); // maximum size

    FluidSim sim;
    sim.initialize(1.0f, size.x, size.y);
    sim.set_boundary(boundary_phi);

    AddParticles(size, sim, boundary_phi);

    sim.add_force(0.01f);
    sim.compute_phi();
    sim.extrapolate_phi();
    sim.apply_projection(0.01f);

    LinearSolver::Data data(*device, size, VMA_MEMORY_USAGE_CPU_ONLY);

    BuildLinearEquation(size, data.Diagonal, data.Lower, data.B, sim);

    LocalGaussSeidel solver(*device, size);

    solver.Bind(data.Diagonal, data.Lower, data.B, data.X);
    ExecuteCommand(*device, [&](vk::CommandBuffer commandBuffer)
    {
        solver.Record(commandBuffer);
    });

    device->Queue().waitIdle();

    CheckPressure(size, sim.pressure, data.X, 1e-1f); // TODO make number of iterations configurable and increase this
}

TEST(LinearSolverTests, Diagonal_Simple_PCG)
{
    glm::ivec2 size(50);

    FluidSim sim;
    sim.initialize(1.0f, size.x, size.y);
    sim.set_boundary(boundary_phi);

    AddParticles(size, sim, boundary_phi);

    sim.add_force(0.01f);
    sim.compute_phi();
    sim.extrapolate_phi();
    sim.apply_projection(0.01f);

    LinearSolver::Data data(*device, size, VMA_MEMORY_USAGE_CPU_ONLY);

    BuildLinearEquation(size, data.Diagonal, data.Lower, data.B, sim);

    Diagonal preconditioner(*device, size);

    LinearSolver::Parameters params(1000, 1e-5f);
    ConjugateGradient solver(*device, size, preconditioner);

    solver.Bind(data.Diagonal, data.Lower, data.B, data.X);
    solver.Solve(params);

    device->Queue().waitIdle();

    CheckPressure(size, sim.pressure, data.X, 1e-5f);

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
    sim.compute_phi();
    sim.extrapolate_phi();
    sim.apply_projection(0.01f);

    LinearSolver::Data data(*device, size, VMA_MEMORY_USAGE_CPU_ONLY);

    BuildLinearEquation(size, data.Diagonal, data.Lower, data.B, sim);

    GaussSeidel preconditioner(*device, size);
    preconditioner.SetW(1.0f);
    preconditioner.SetPreconditionerIterations(8);

    LinearSolver::Parameters params(1000, 1e-5f);
    ConjugateGradient solver(*device, size, preconditioner);

    solver.Bind(data.Diagonal, data.Lower, data.B, data.X);
    solver.Solve(params);

    device->Queue().waitIdle();

    CheckPressure(size, sim.pressure, data.X, 1e-4f);

    std::cout << "Solved with number of iterations: " << params.OutIterations << std::endl;
}

TEST(LinearSolverTests, IncompletePoisson_Simple_PCG)
{
    glm::ivec2 size(50);

    FluidSim sim;
    sim.initialize(1.0f, size.x, size.y);
    sim.set_boundary(boundary_phi);

    AddParticles(size, sim, boundary_phi);

    sim.add_force(0.01f);
    sim.compute_phi();
    sim.extrapolate_phi();
    sim.apply_projection(0.01f);

    LinearSolver::Data data(*device, size, VMA_MEMORY_USAGE_CPU_ONLY);

    BuildLinearEquation(size, data.Diagonal, data.Lower, data.B, sim);

    IncompletePoisson preconditioner(*device, size);

    LinearSolver::Parameters params(1000, 1e-5f);
    ConjugateGradient solver(*device, size, preconditioner);

    solver.Bind(data.Diagonal, data.Lower, data.B, data.X);
    solver.Solve(params);

    device->Queue().waitIdle();

    CheckPressure(size, sim.pressure, data.X, 1e-5f);

    std::cout << "Solved with number of iterations: " << params.OutIterations << std::endl;
}

TEST(LinearSolverTests, Zero_PCG)
{
    glm::ivec2 size(50);

    LinearSolver::Data data(*device, size, VMA_MEMORY_USAGE_CPU_ONLY);

    Diagonal preconditioner(*device, size);

    LinearSolver::Parameters params(1000, 1e-5f);
    ConjugateGradient solver(*device, size, preconditioner);

    solver.Bind(data.Diagonal, data.Lower, data.B, data.X);
    solver.Solve(params);

    device->Queue().waitIdle();

    ASSERT_EQ(0, params.OutIterations);

    std::vector<float> pressureData(size.x*size.y, 1.0f);
    CopyTo(data.X, pressureData);

    for (int i = 0; i < size.x; i++)
    {
        for (int j = 0; j < size.y; j++)
        {
            EXPECT_FLOAT_EQ(0.0f, pressureData[i + size.x * j]);
        }
    }
}

TEST(LinearSolverTests, Multigrid_Simple_PCG)
{
    glm::ivec2 size(64);

    FluidSim sim;
    sim.initialize(1.0f, size.x, size.y);
    sim.set_boundary(boundary_phi);

    AddParticles(size, sim, boundary_phi);

    sim.add_force(0.01f);
    sim.compute_phi();
    sim.extrapolate_phi();
    sim.apply_projection(0.01f);

    LinearSolver::Data data(*device, size, VMA_MEMORY_USAGE_CPU_ONLY);

    Velocity velocity(*device, size);
    Texture liquidPhi(*device, size.x, size.y, vk::Format::eR32Sfloat);
    Texture solidPhi(*device, size.x, size.y, vk::Format::eR32Sfloat);
    Buffer<glm::ivec2> valid(*device, size.x*size.y, VMA_MEMORY_USAGE_CPU_ONLY);

    SetSolidPhi(*device, size, solidPhi, sim, (float)size.x);
    SetLiquidPhi(*device, size, liquidPhi, sim, (float)size.x);

    BuildLinearEquation(size, data.Diagonal, data.Lower, data.B, sim);

    Pressure pressure(*device, 0.01f, size, data, velocity, solidPhi, liquidPhi, valid);

    Multigrid preconditioner(*device, size, 0.01f);
    preconditioner.BuildHierarchiesBind(pressure, solidPhi, liquidPhi);

    LinearSolver::Parameters params(1000, 1e-5f);
    ConjugateGradient solver(*device, size, preconditioner);

    solver.Bind(data.Diagonal, data.Lower, data.B, data.X);

    preconditioner.BuildHierarchies();
    solver.Solve(params);

    device->Queue().waitIdle();

    CheckPressure(size, sim.pressure, data.X, 1e-5f);

    std::cout << "Solved with number of iterations: " << params.OutIterations << std::endl;
}
