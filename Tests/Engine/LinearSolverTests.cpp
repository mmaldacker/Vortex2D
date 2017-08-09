//
//  LinearSolverTests.cpp
//  Vortex2D
//

#include "Verify.h"
#include "VariationalHelpers.h"
#include <Vortex2D/Engine/LinearSolver/Reduce.h>
#include <Vortex2D/Engine/LinearSolver/GaussSeidel.h>
#include <Vortex2D/Engine/LinearSolver/Transfer.h>

/*
#include <Vortex2D/Engine/LinearSolver/ConjugateGradient.h>
#include <Vortex2D/Engine/LinearSolver/Multigrid.h>
#include <Vortex2D/Engine/Pressure.h>
*/
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

    ReduceSum reduce(*device, size, input, output);

    std::vector<float> inputData(n);

    {
        float n = 1.0f;
        std::generate(inputData.begin(), inputData.end(), [&n]{ return n++; });
    }

    input.CopyFrom(inputData);

    reduce.Submit();
    device->Handle().waitIdle();

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

    ReduceSum reduce(*device, size, input, output);

    std::vector<float> inputData(n, 1.0f);

    {
        float n = 1.0f;
        std::generate(inputData.begin(), inputData.end(), [&n]{ return n++; });
    }

    input.CopyFrom(inputData);

    reduce.Submit();
    device->Handle().waitIdle();

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

    ReduceMax reduce(*device, size, input, output);

    std::vector<float> inputData(10*15);

    {
        float n = -1.0f;
        std::generate(inputData.begin(), inputData.end(), [&n]{ return n--; });
    }

    input.CopyFrom(inputData);

    reduce.Submit();
    device->Handle().waitIdle();

    std::vector<float> outputData(1, 0.0f);
    output.CopyTo(outputData);

    ASSERT_EQ(150.0f, outputData[0]);
}

TEST(LinearSolverTests, Transfer_Prolongate)
{
    glm::ivec2 coarseSize(4);
    glm::ivec2 fineSize(6);

    Transfer t(*device);

    Buffer input(*device, vk::BufferUsageFlagBits::eStorageBuffer, true, sizeof(float)*coarseSize.x*coarseSize.y);
    Buffer output(*device, vk::BufferUsageFlagBits::eStorageBuffer, true, sizeof(float)*fineSize.x*fineSize.y);

    std::vector<float> data(coarseSize.x * coarseSize.y, 0.0f);
    std::iota(data.begin(), data.end(), 1.0f);
    input.CopyFrom(data);

    t.Init(coarseSize, input, output);
    t.Prolongate(0);
    device->Queue().waitIdle();

    std::vector<float> outputData(fineSize.x*fineSize.y, 0.0f);
    output.CopyTo(outputData);

    float total;
    total = (9*6 + 3*7 + 3*10 + 1*11) / 16.0f;
    EXPECT_FLOAT_EQ(total, outputData[1 + 6 * 1]);

    total = (9*7 + 3*6 + 3*11 + 1*10) / 16.0f;
    EXPECT_FLOAT_EQ(total, outputData[2 + 6 * 1]);

    total = (9*10 + 3*6 + 3*11 + 1*7) / 16.0f;
    EXPECT_FLOAT_EQ(total, outputData[1 + 6 * 2]);

    total = (9*11 + 3*7 + 3*10 + 1*6) / 16.0f;
    EXPECT_FLOAT_EQ(total, outputData[2 + 6 * 2]);
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

    t.Init(coarseSize, output, input);
    t.Restrict(0);
    device->Queue().waitIdle();

    float total = (1*1 + 3*2 + 3*3 + 1*4 +
                   3*5 + 9*6 + 9*7 + 3*8 +
                   3*9 + 9*10 + 9*11 + 3*12 +
                   1*13 + 3*14 + 3*15 + 1*16) / 64.0f;

    std::vector<float> outputData(coarseSize.x*coarseSize.y, 0.0f);
    output.CopyTo(outputData);

    EXPECT_FLOAT_EQ(total, outputData[1 + 3 * 1]);
}

void BuildLinearEquation(const glm::ivec2& size, Buffer& matrix, Buffer& div, FluidSim& sim)
{
    std::vector<LinearSolver::Data> matrixData(size.x * size.y);
    std::vector<float> divData(size.x * size.y);

    for (int i = 1; i < size.x - 1; i++)
    {
        for (int j = 1; j < size.y - 1; j++)
        {
            unsigned index = i + size.x * j;
            divData[index] = (float)sim.rhs[index];
            matrixData[index].Diagonal = (float)sim.matrix(index, index);
            matrixData[index].Weights.x = (float)sim.matrix(index + 1, index);
            matrixData[index].Weights.y = (float)sim.matrix(index - 1, index);
            matrixData[index].Weights.z = (float)sim.matrix(index, index + size.x);
            matrixData[index].Weights.w = (float)sim.matrix(index, index - size.x);
        }
    }

    matrix.CopyFrom(matrixData);
    div.CopyFrom(divData);
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

    LinearSolver::Parameters params(200);
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

    LinearSolver::Parameters params(200);
    GaussSeidel solver(*device, size);

    solver.Init(matrix, div, pressure);
    solver.Solve(params);

    device->Queue().waitIdle();

    CheckPressure(size, sim.pressure, pressure, 1e-4f);

    std::cout << "Solved with number of iterations: " << params.OutIterations << std::endl;
}

/*
TEST(LinearSolverTests, Simple_CG)
{
    Disable d(GL_BLEND);

    // FIXME setting size 20 doesn't work???
    glm::vec2 size(50);

    FluidSim sim;
    sim.initialize(1.0f, size.x, size.y);
    sim.set_boundary(boundary_phi);

    AddParticles(size, sim, boundary_phi);

    sim.add_force(0.01f);
    sim.project(0.01f);

    LinearSolver::Data data(size);
    BuildLinearEquation(size, data, sim);

    // FIXME error tolerance doesn't work here
    LinearSolver::Parameters params(600);
    ConjugateGradient solver(size);
    solver.Init(data);
    solver.NormalSolve(data, params);

    Reader reader(data.Pressure);
    reader.Read();

    CheckPressure(size, sim.pressure, data, 1e-4f);

    std::cout << "Solved with number of iterations: " << params.OutIterations << std::endl;
}

TEST(LinearSolverTests, Simple_PCG)
{
    Disable d(GL_BLEND);

    glm::vec2 size(50);

    FluidSim sim;
    sim.initialize(1.0f, size.x, size.y);
    sim.set_boundary(boundary_phi);

    AddParticles(size, sim, boundary_phi);

    sim.add_force(0.01f);

    Buffer velocity1(size, 2, true);
    SetVelocity(velocity1, sim);

    Buffer velocity2(size, 2, true);
    SetVelocity(velocity2, sim);

    sim.project(0.01f);

    Buffer solidPhi(glm::vec2(2)*size, 1);
    SetSolidPhi(solidPhi, sim);

    Buffer liquidPhi(size, 1);
    SetLiquidPhi(liquidPhi, sim);

    Buffer solidVelocity(size, 2);
    // leave empty

    LinearSolver::Data data(size);

    LinearSolver::Parameters params(1000, 1e-5f);
    ConjugateGradient solver(size);

    Pressure pressure(0.01f, size, solver, data, velocity1, solidPhi, liquidPhi, solidVelocity);

    auto start = std::chrono::system_clock::now();

    pressure.Solve(params);

    auto end = std::chrono::system_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    Reader reader(data.Pressure);
    reader.Read();

    // FIXME error tolerance doesn't work here (see comment in CG)
    CheckPressure(size, sim.pressure, data, 1e-3f);

    std::cout << "Solved in time: " << elapsed.count() << std::endl;
    std::cout << "Solved with number of iterations: " << params.OutIterations << std::endl;
}

TEST(LinearSolverTests, Complex_PCG)
{
    Disable d(GL_BLEND);

    glm::vec2 size(50);

    FluidSim sim;
    sim.initialize(1.0f, size.x, size.y);
    sim.set_boundary(boundary_phi);

    AddParticles(size, sim, boundary_phi);

    sim.add_force(0.01f);

    Buffer velocity1(size, 2, true);
    SetVelocity(velocity1, sim);

    Buffer velocity2(size, 2, true);
    SetVelocity(velocity2, sim);

    sim.project(0.01f);

    Buffer solidPhi(glm::vec2(2)*size, 1);
    SetSolidPhi(solidPhi, sim);

    Buffer liquidPhi(size, 1);
    SetLiquidPhi(liquidPhi, sim);

    Buffer solidVelocity(size, 2);
    // leave empty

    LinearSolver::Data data(size);

    LinearSolver::Parameters params(1000, 1e-5f);
    ConjugateGradient solver(size);

    Pressure pressure(0.01f, size, solver, data, velocity1, solidPhi, liquidPhi, solidVelocity);

    auto start = std::chrono::system_clock::now();

    pressure.Solve(params);

    auto end = std::chrono::system_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    Reader reader(data.Pressure);
    reader.Read().Print();

    // FIXME error tolerance doesn't work here (see comment in CG)
    CheckPressure(size, sim.pressure, data, 1e-3f);

    std::cout << "Solved in time: " << elapsed.count() << std::endl;
    std::cout << "Solved with number of iterations: " << params.OutIterations << std::endl;
}

TEST(LinearSolverTests, Zero_CG)
{
    Disable d(GL_BLEND);

    glm::vec2 size(50);

    LinearSolver::Data data(size);
    data.Pressure.Clear(glm::vec4(0.0f));

    LinearSolver::Parameters params(1000, 1e-5f);
    ConjugateGradient solver(size);
    solver.Init(data);
    solver.NormalSolve(data, params);

    Reader reader(data.Pressure);
    reader.Read();

    for (int i = 0; i < size.x; i++)
    {
        for (int j = 0; j < size.y; j++)
        {
            EXPECT_FLOAT_EQ(0.0f, reader.GetVec2(i, j).x);
        }
    }
}


TEST(LinearSolverTests, Zero_PCG)
{
    Disable d(GL_BLEND);

    glm::vec2 size(50);

    LinearSolver::Data data(size);
    data.Pressure.Clear(glm::vec4(0.0f));

    LinearSolver::Parameters params(1000, 1e-5f);
    ConjugateGradient solver(size);
    solver.Init(data);
    solver.Solve(data, params);

    Reader reader(data.Pressure);
    reader.Read();

    for (int i = 0; i < size.x; i++)
    {
        for (int j = 0; j < size.y; j++)
        {
            EXPECT_FLOAT_EQ(0.0f, reader.GetVec2(i, j).x);
        }
    }
}

TEST(LinearSolverTests, Simple_Multigrid)
{
    Disable d(GL_BLEND);

    glm::vec2 size(16);

    FluidSim sim;
    sim.initialize(1.0f, size.x, size.y);
    sim.set_boundary(boundary_phi);

    AddParticles(size, sim, boundary_phi);

    sim.add_force(0.01f);

    Buffer velocity1(size, 2, true);
    SetVelocity(velocity1, sim);

    Buffer velocity2(size, 2, true);
    SetVelocity(velocity2, sim);

    sim.project(0.01f);

    Buffer solidPhi(glm::vec2(2)*size, 1);
    SetSolidPhi(solidPhi, sim);

    Buffer liquidPhi(size, 1);
    SetLiquidPhi(liquidPhi, sim);

    Buffer solidVelocity(size, 2);
    // leave empty

    // multigrid solver
    {
        LinearSolver::Data data(size);

        LinearSolver::Parameters params(0);
        Multigrid solver(size);

        Pressure pressure(0.01f, size, solver, data, velocity1, solidPhi, liquidPhi, solidVelocity);
        pressure.Solve(params);

        Reader(data.Pressure).Read().Print();
    }

    // solution from SOR with only few iterations (to check multigrid is an improvement)
    {
        LinearSolver::Data data(size);

        LinearSolver::Parameters params(4);
        GaussSeidel solver(size);

        Pressure pressure(0.01f, size, solver, data, velocity2, solidPhi, liquidPhi, solidVelocity);
        pressure.Solve(params);

        Reader(data.Pressure).Read().Print();
    }

    // solution from FluidSim
    {
        PrintData(size.x, size.y, sim.pressure);
    }
}
*/
