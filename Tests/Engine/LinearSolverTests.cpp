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
    glm::ivec2 size(500);
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

TEST(LinearSolverTests, Transfer_Prolongate)
{

    glm::ivec2 coarseSize(3);
    glm::ivec2 fineSize(4);

    Transfer t(*device);

    Buffer diagonal(*device, vk::BufferUsageFlagBits::eStorageBuffer, true, fineSize.x*fineSize.y*sizeof(float));
    std::vector<float> diagonalData(fineSize.x*fineSize.y, {1.0f});
    diagonal.CopyFrom(diagonalData);

    Buffer input(*device, vk::BufferUsageFlagBits::eStorageBuffer, true, sizeof(float)*coarseSize.x*coarseSize.y);
    Buffer output(*device, vk::BufferUsageFlagBits::eStorageBuffer, true, sizeof(float)*fineSize.x*fineSize.y);

    std::vector<float> data(coarseSize.x * coarseSize.y, 0.0f);
    std::iota(data.begin(), data.end(), 1.0f);
    input.CopyFrom(data);

    t.InitProlongate(fineSize, output, input, diagonal);
    ExecuteCommand(*device, [&](vk::CommandBuffer commandBuffer)
    {
        t.Prolongate(commandBuffer, 0);
    });

    std::vector<float> outputData(fineSize.x*fineSize.y, 0.0f);
    output.CopyTo(outputData);

    float total;
    total = (9*5 + 3*2 + 3*4 + 1*1) / 16.0f;
    EXPECT_FLOAT_EQ(total, outputData[1 + fineSize.x * 1]);

    total = (9*5 + 3*2 + 3*6 + 1*3) / 16.0f;
    EXPECT_FLOAT_EQ(total, outputData[2 + fineSize.x * 1]);

    total = (9*5 + 3*4 + 3*8 + 1*7) / 16.0f;
    EXPECT_FLOAT_EQ(total, outputData[1 + fineSize.x * 2]);

    total = (9*5 + 3*6 + 3*8 + 1*9) / 16.0f;
    EXPECT_FLOAT_EQ(total, outputData[2 + fineSize.x * 2]);

}

TEST(LinearSolverTests, Transfer_Restrict)
{

    glm::ivec2 coarseSize(3);
    glm::ivec2 fineSize(4);

    Transfer t(*device);

    Buffer input(*device, vk::BufferUsageFlagBits::eStorageBuffer, true, sizeof(float)*fineSize.x*fineSize.y);
    Buffer output(*device, vk::BufferUsageFlagBits::eStorageBuffer, true, sizeof(float)*coarseSize.x*coarseSize.y);

    std::vector<float> data(fineSize.x * fineSize.y, 1.0f);
    std::iota(data.begin(), data.end(), 1.0f);
    input.CopyFrom(data);

    t.InitRestrict(fineSize, input, output);
    ExecuteCommand(*device, [&](vk::CommandBuffer commandBuffer)
    {
        t.Restrict(commandBuffer, 0);
    });

    float total = (1*1 + 3*2 + 3*3 + 1*4 +
                   3*5 + 9*6 + 9*7 + 3*8 +
                   3*9 + 9*10 + 9*11 + 3*12 +
                   1*13 + 3*14 + 3*15 + 1*16) / 64.0f;

    std::vector<float> outputData(coarseSize.x*coarseSize.y, 0.0f);
    output.CopyTo(outputData);

    EXPECT_FLOAT_EQ(total, outputData[1 + coarseSize.x * 1]);
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

    Buffer diagonal(*device, vk::BufferUsageFlagBits::eStorageBuffer, true, size.x*size.y*sizeof(float));
    Buffer lower(*device, vk::BufferUsageFlagBits::eStorageBuffer, true, size.x*size.y*sizeof(glm::vec2));
    Buffer div(*device, vk::BufferUsageFlagBits::eStorageBuffer, true, size.x*size.y*sizeof(float));
    Buffer pressure(*device, vk::BufferUsageFlagBits::eStorageBuffer, true, size.x*size.y*sizeof(float));

    BuildLinearEquation(size, diagonal, lower, div, sim);

    LinearSolver::Parameters params(1000, 1e-4f);
    GaussSeidel solver(*device, size);

    solver.Init(diagonal, lower, div, pressure);
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

    Buffer diagonal(*device, vk::BufferUsageFlagBits::eStorageBuffer, true, size.x*size.y*sizeof(float));
    Buffer lower(*device, vk::BufferUsageFlagBits::eStorageBuffer, true, size.x*size.y*sizeof(glm::vec2));
    Buffer div(*device, vk::BufferUsageFlagBits::eStorageBuffer, true, size.x*size.y*sizeof(float));
    Buffer pressure(*device, vk::BufferUsageFlagBits::eStorageBuffer, true, size.x*size.y*sizeof(float));

    BuildLinearEquation(size, diagonal, lower, div, sim);

    LinearSolver::Parameters params(1000, 1e-4f);
    GaussSeidel solver(*device, size);

    solver.Init(diagonal, lower, div, pressure);
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

    Buffer diagonal(*device, vk::BufferUsageFlagBits::eStorageBuffer, true, size.x*size.y*sizeof(float));
    Buffer lower(*device, vk::BufferUsageFlagBits::eStorageBuffer, true, size.x*size.y*sizeof(glm::vec2));
    Buffer div(*device, vk::BufferUsageFlagBits::eStorageBuffer, true, size.x*size.y*sizeof(float));
    Buffer pressure(*device, vk::BufferUsageFlagBits::eStorageBuffer, true, size.x*size.y*sizeof(float));

    BuildLinearEquation(size, diagonal, lower, div, sim);

    Diagonal preconditioner(*device, size);

    LinearSolver::Parameters params(1000, 1e-5f);
    ConjugateGradient solver(*device, size, preconditioner);

    solver.Init(diagonal, lower, div, pressure);
    solver.NormalSolve(params);

    device->Queue().waitIdle();

    CheckPressure(size, sim.pressure, pressure, 1e-3f); // TODO somehow error is bigger than 1e-5

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

    Buffer diagonal(*device, vk::BufferUsageFlagBits::eStorageBuffer, true, size.x*size.y*sizeof(float));
    Buffer lower(*device, vk::BufferUsageFlagBits::eStorageBuffer, true, size.x*size.y*sizeof(glm::vec2));
    Buffer div(*device, vk::BufferUsageFlagBits::eStorageBuffer, true, size.x*size.y*sizeof(float));
    Buffer pressure(*device, vk::BufferUsageFlagBits::eStorageBuffer, true, size.x*size.y*sizeof(float));

    BuildLinearEquation(size, diagonal, lower, div, sim);

    Diagonal preconditioner(*device, size);

    LinearSolver::Parameters params(1000, 1e-5f);
    ConjugateGradient solver(*device, size, preconditioner);

    solver.Init(diagonal, lower, div, pressure);
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

    Buffer diagonal(*device, vk::BufferUsageFlagBits::eStorageBuffer, true, size.x*size.y*sizeof(float));
    Buffer lower(*device, vk::BufferUsageFlagBits::eStorageBuffer, true, size.x*size.y*sizeof(glm::vec2));
    Buffer div(*device, vk::BufferUsageFlagBits::eStorageBuffer, true, size.x*size.y*sizeof(float));
    Buffer pressure(*device, vk::BufferUsageFlagBits::eStorageBuffer, true, size.x*size.y*sizeof(float));

    BuildLinearEquation(size, diagonal, lower, div, sim);

    GaussSeidel preconditioner(*device, size);
    preconditioner.SetW(1.0f);
    preconditioner.SetPreconditionerIterations(8);

    LinearSolver::Parameters params(1000, 1e-5f);
    ConjugateGradient solver(*device, size, preconditioner);

    solver.Init(diagonal, lower, div, pressure);
    solver.Solve(params);

    device->Queue().waitIdle();

    CheckPressure(size, sim.pressure, pressure, 1e-4f);

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
    sim.project(0.01f);

    Buffer diagonal(*device, vk::BufferUsageFlagBits::eStorageBuffer, true, size.x*size.y*sizeof(float));
    Buffer lower(*device, vk::BufferUsageFlagBits::eStorageBuffer, true, size.x*size.y*sizeof(glm::vec2));
    Buffer div(*device, vk::BufferUsageFlagBits::eStorageBuffer, true, size.x*size.y*sizeof(float));
    Buffer pressure(*device, vk::BufferUsageFlagBits::eStorageBuffer, true, size.x*size.y*sizeof(float));

    BuildLinearEquation(size, diagonal, lower, div, sim);

    IncompletePoisson preconditioner(*device, size);

    LinearSolver::Parameters params(1000, 1e-5f);
    ConjugateGradient solver(*device, size, preconditioner);

    solver.Init(diagonal, lower, div, pressure);
    solver.Solve(params);

    device->Queue().waitIdle();

    CheckPressure(size, sim.pressure, pressure, 1e-5f);

    std::cout << "Solved with number of iterations: " << params.OutIterations << std::endl;
}

TEST(LinearSolverTests, Zero_CG)
{
    glm::vec2 size(50);

    Buffer diagonal(*device, vk::BufferUsageFlagBits::eStorageBuffer, true, size.x*size.y*sizeof(float));
    Buffer lower(*device, vk::BufferUsageFlagBits::eStorageBuffer, true, size.x*size.y*sizeof(glm::vec2));
    Buffer div(*device, vk::BufferUsageFlagBits::eStorageBuffer, true, size.x*size.y*sizeof(float));
    Buffer pressure(*device, vk::BufferUsageFlagBits::eStorageBuffer, true, size.x*size.y*sizeof(float));

    Diagonal preconditioner(*device, size);

    LinearSolver::Parameters params(1000, 1e-5f);
    ConjugateGradient solver(*device, size, preconditioner);

    solver.Init(diagonal, lower, div, pressure);
    solver.NormalSolve(params);

    device->Queue().waitIdle();

    ASSERT_EQ(0, params.OutIterations);

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

    Buffer diagonal(*device, vk::BufferUsageFlagBits::eStorageBuffer, true, size.x*size.y*sizeof(float));
    Buffer lower(*device, vk::BufferUsageFlagBits::eStorageBuffer, true, size.x*size.y*sizeof(glm::vec2));
    Buffer div(*device, vk::BufferUsageFlagBits::eStorageBuffer, true, size.x*size.y*sizeof(float));
    Buffer pressure(*device, vk::BufferUsageFlagBits::eStorageBuffer, true, size.x*size.y*sizeof(float));

    Diagonal preconditioner(*device, size);

    LinearSolver::Parameters params(1000, 1e-5f);
    ConjugateGradient solver(*device, size, preconditioner);

    solver.Init(diagonal, lower, div, pressure);
    solver.Solve(params);

    device->Queue().waitIdle();

    ASSERT_EQ(0, params.OutIterations);

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

TEST(LinearSolverTests, Simple_Multigrid)
{
    glm::ivec2 size(16);

    FluidSim sim;
    sim.initialize(1.0f, size.x, size.y);
    sim.set_boundary(boundary_phi);

    AddParticles(size, sim, boundary_phi);

    sim.add_force(0.01f);
    sim.project(0.01f);

    // solution from FluidSim
    PrintData(size.x, size.y, sim.pressure);

    LinearSolver::Data data(*device, size, true);

    Texture velocity(*device, size.x, size.y, vk::Format::eR32G32Sfloat, false);
    Texture solidPhi(*device, size.x, size.y, vk::Format::eR32Sfloat, false);
    Texture liquidPhi(*device, size.x, size.y, vk::Format::eR32Sfloat, false);
    Texture solidVelocity(*device, size.x, size.y, vk::Format::eR32G32Sfloat, false);
    Buffer valid(*device, vk::BufferUsageFlagBits::eStorageBuffer, true, size.x*size.y*sizeof(glm::ivec2));

    Texture inputSolidPhi(*device, size.x, size.y, vk::Format::eR32Sfloat, true);
    SetSolidPhi(size, inputSolidPhi, sim, size.x);

    Texture inputLiquidPhi(*device, size.x, size.y, vk::Format::eR32Sfloat, true);
    SetLiquidPhi(size, inputLiquidPhi, sim, size.x);

    Vortex2D::Renderer::ExecuteCommand(*device, [&](vk::CommandBuffer commandBuffer)
    {
        solidPhi.CopyFrom(commandBuffer, inputSolidPhi);
        liquidPhi.CopyFrom(commandBuffer, inputLiquidPhi);
    });

    BuildLinearEquation(size, data.Diagonal, data.Lower, data.B, sim);

    Pressure pressure(*device, 0.01f, size, data, velocity, solidPhi, liquidPhi, solidVelocity, valid);

    Multigrid multigrid(*device, size, 0.01f);
    multigrid.Build(pressure, solidPhi, liquidPhi);
    multigrid.Init(data.Diagonal, data.Lower, data.B, data.X);

    ExecuteCommand(*device, [&](vk::CommandBuffer commandBuffer)
    {
        multigrid.RecordInit(commandBuffer);
        multigrid.Record(commandBuffer);
    });

    device->Queue().waitIdle();

    Texture localLiquidPhi(*device, 16, 16, vk::Format::eR32Sfloat, true);
    Texture localLiquidPhi2(*device, 9, 9, vk::Format::eR32Sfloat, true);

    ExecuteCommand(*device, [&](vk::CommandBuffer commandBuffer)
    {
        localLiquidPhi.CopyFrom(commandBuffer, liquidPhi);
        localLiquidPhi2.CopyFrom(commandBuffer, multigrid.mLiquidPhis[0]);
    });

    PrintTexture<float>(localLiquidPhi);
    PrintTexture<float>(localLiquidPhi2);

    PrintBuffer<float>(size, data.X);
}

TEST(LinearSolverTests, Multigrid_Simple_PCG)
{
    glm::ivec2 size(66);

    FluidSim sim;
    sim.initialize(1.0f, size.x, size.y);
    sim.set_boundary(boundary_phi);

    AddParticles(size, sim, boundary_phi);

    sim.add_force(0.01f);
    sim.project(0.01f);

    LinearSolver::Data data(*device, size, true);

    Texture velocity(*device, size.x, size.y, vk::Format::eR32G32Sfloat, false);
    Texture liquidPhi(*device, size.x, size.y, vk::Format::eR32Sfloat, false);
    Texture solidPhi(*device, size.x, size.y, vk::Format::eR32Sfloat, false);
    Texture solidVelocity(*device, size.x, size.y, vk::Format::eR32G32Sfloat, false);
    Buffer valid(*device, vk::BufferUsageFlagBits::eStorageBuffer, true, size.x*size.y*sizeof(glm::ivec2));

    Texture inputSolidPhi(*device, size.x, size.y, vk::Format::eR32Sfloat, true);
    SetSolidPhi(size, inputSolidPhi, sim, size.x);

    Texture inputLiquidPhi(*device, size.x, size.y, vk::Format::eR32Sfloat, true);
    SetLiquidPhi(size, inputLiquidPhi, sim, size.x);

    Vortex2D::Renderer::ExecuteCommand(*device, [&](vk::CommandBuffer commandBuffer)
    {
        solidPhi.CopyFrom(commandBuffer, inputSolidPhi);
        liquidPhi.CopyFrom(commandBuffer, inputLiquidPhi);
    });

    BuildLinearEquation(size, data.Diagonal, data.Lower, data.B, sim);

    Pressure pressure(*device, 0.01f, size, data, velocity, solidPhi, liquidPhi, solidVelocity, valid);

    Multigrid preconditioner(*device, size, 0.01f);
    preconditioner.Build(pressure, solidPhi, liquidPhi);

    LinearSolver::Parameters params(1000, 1e-5f);
    ConjugateGradient solver(*device, size, preconditioner);

    solver.Init(data.Diagonal, data.Lower, data.B, data.X);
    solver.Solve(params);

    device->Queue().waitIdle();

    CheckPressure(size, sim.pressure, data.X, 1e-5f);

    std::cout << "Solved with number of iterations: " << params.OutIterations << std::endl;
}
