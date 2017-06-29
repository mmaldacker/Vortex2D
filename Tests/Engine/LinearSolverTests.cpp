//
//  LinearSolverTests.cpp
//  Vortex2D
//

#include "Verify.h"
#include <Vortex2D/Engine/LinearSolver/Reduce.h>

/*
#include <Vortex2D/Engine/LinearSolver/ConjugateGradient.h>
#include <Vortex2D/Engine/LinearSolver/GaussSeidel.h>
#include <Vortex2D/Engine/LinearSolver/Multigrid.h>
#include <Vortex2D/Engine/LinearSolver/Transfer.h>
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
    glm::vec2 size(10, 15);
    float n = size.x * size.y;
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
    glm::vec2 size(500, 500);
    float n = size.x * size.y; // 1 million

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
    glm::vec2 size(10, 15);
    float n = size.x * size.y;

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

/*
TEST(LinearSolverTests, Transfer_Prolongate)
{
    Disable d(GL_BLEND);

    glm::vec2 size(2);

    Transfer t;

    Buffer pressure(size, 2);
    pressure.Clear(glm::vec4(0.0f));

    Buffer input(size, 1), output(glm::vec2(2) * size, 2);

    std::vector<float> data(size.x * size.y, 0.0f);
    std::iota(data.begin(), data.end(), 1.0f);
    Writer(input).Write(data);

    output = t.Prolongate(input, pressure);

    float total;

    total = (9*1 + 3*2 + 3*3 + 1*4) / 16.0f;
    EXPECT_FLOAT_EQ(total, Reader(output).Read().GetVec2(1, 1).x);

    total = (9*2 + 3*1 + 3*4 + 1*3) / 16.0f;
    EXPECT_FLOAT_EQ(total, Reader(output).Read().GetVec2(2, 1).x);

    total = (9*3 + 3*1 + 3*4 + 1*2) / 16.0f;
    EXPECT_FLOAT_EQ(total, Reader(output).Read().GetVec2(1, 2).x);

    total = (9*4 + 3*2 + 3*3 + 1*1) / 16.0f;
    EXPECT_FLOAT_EQ(total, Reader(output).Read().GetVec2(2, 2).x);
}

TEST(LinearSolverTests, Transfer_Restrict)
{
    Disable d(GL_BLEND);

    glm::vec2 size(3);

    Transfer t;

    Buffer input(glm::vec2(2) * size, 1), output(size, 2);

    std::vector<float> data(size.x * size.y * 4, 1.0f);
    std::iota(data.begin(), data.end(), 1.0f);
    Writer(input).Write(data);

    output = t.Restrict(input);

    float total = (1*8 + 3*9 + 3*10 + 1*11 +
            3*14 + 9*15 + 9*16 + 3*17 +
            3*20 + 9*21 + 9*22 + 3*23 +
            1*26 + 3*27 + 3*28 + 1*29) / 64.0f;

    EXPECT_FLOAT_EQ(total, Reader(output).Read().GetVec2(1, 1).y);
}

TEST(LinearSolverTests, RenderMask)
{
    Disable d(GL_BLEND);

    glm::vec2 size(10);

    Buffer buffer(size, 1, true, true);
    LinearSolver::Data data(size);

    std::vector<float> dataData(size.x * size.y, 0.0f);
    dataData[15] = 1.0f;
    Writer(data.Diagonal).Write(dataData);

    NiceMock<MockLinearSolver> solver;
    solver.RenderMask(buffer, data);

    Reader reader(buffer);
    reader.ReadStencil();

    for (std::size_t i = 0; i < size.x; i++)
    {
        for (std::size_t j = 0; j < size.y; j++)
        {
            uint8_t value = dataData[i + j * size.x];
            EXPECT_EQ(value, reader.GetStencil(i, j)) << "Value not equal at " << i << ", " << j;
        }
    }
}

void BuildLinearEquation(const glm::vec2& size, LinearSolver::Data& data, FluidSim& sim)
{
    std::vector<glm::vec2> pressureData(size.x * size.y, glm::vec2(0.0f));
    for (std::size_t i = 0; i < pressureData.size(); i++)
    {
        pressureData[i].y = sim.rhs[i];
    }

    std::vector<float> diagonalData(size.x * size.y, 0.0f);
    for (std::size_t index = 0; index < diagonalData.size(); index++)
    {
        diagonalData[index] = sim.matrix(index, index);
    }

    std::vector<glm::vec4> weightsData(size.x * size.y, glm::vec4(0.0f));
    for (std::size_t i = 1; i < size.x - 1; i++)
    {
        for (std::size_t j = 1; j < size.y - 1; j++)
        {
            std::size_t index = i + size.x * j;
            weightsData[index].x = sim.matrix(index + 1, index);
            weightsData[index].y = sim.matrix(index - 1, index);
            weightsData[index].z = sim.matrix(index, index + size.x);
            weightsData[index].w = sim.matrix(index, index - size.x);
        }
    }

    Writer(data.Pressure).Write(pressureData);
    Writer(data.Diagonal).Write(diagonalData);
    Writer(data.Weights).Write(weightsData);
}

void CheckPressure(const glm::vec2& size, const std::vector<double>& pressure, LinearSolver::Data& data, float error)
{
    Reader reader(data.Pressure);
    reader.Read();

    for (int i = 0; i < size.x; i++)
    {
        for (int j = 0; j < size.y; j++)
        {
            std::size_t index = i + j * size.x;
            float value = pressure[index];
            EXPECT_NEAR(value, reader.GetVec2(i, j).x, error) << "Mismatch at " << i << ", " << j << "\n";
        }
    }
}

TEST(LinearSolverTests, Simple_SOR)
{
    Disable d(GL_BLEND);

    glm::vec2 size(50);

    FluidSim sim;
    sim.initialize(1.0f, size.x, size.y);
    sim.set_boundary(boundary_phi);

    AddParticles(size, sim, boundary_phi);

    sim.add_force(0.01f);
    sim.project(0.01f);

    LinearSolver::Data data(size);
    BuildLinearEquation(size, data, sim);

    LinearSolver::Parameters params(200);
    GaussSeidel solver(size);

    solver.Init(data);
    solver.Solve(data, params);

    CheckPressure(size, sim.pressure, data, 1e-4f);

    std::cout << "Solved with number of iterations: " << params.OutIterations << std::endl;
}

TEST(LinearSolverTests, Complex_SOR)
{
    Disable d(GL_BLEND);

    glm::vec2 size(50);

    FluidSim sim;
    sim.initialize(1.0f, size.x, size.y);
    sim.set_boundary(complex_boundary_phi);

    AddParticles(size, sim, complex_boundary_phi);

    sim.add_force(0.01f);
    sim.project(0.01f);

    LinearSolver::Data data(size);
    BuildLinearEquation(size, data, sim);

    LinearSolver::Parameters params(200);
    GaussSeidel solver(size);

    solver.Init(data);
    solver.Solve(data, params);

    CheckPressure(size, sim.pressure, data, 1e-4f);

    std::cout << "Solved with number of iterations: " << params.OutIterations << std::endl;
}

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

TEST(LinearSolverTests, PerformanceMeasurements)
{
    Disable d(GL_BLEND);

    glm::vec2 size(100);

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

    std::cout << "Total Solved time: " << elapsed.count() << std::endl;
    std::cout << "Solved with number of iterations: " << params.OutIterations << std::endl;
}
*/
