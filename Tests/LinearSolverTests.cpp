//
//  LinearSolverTests.cpp
//  Vortex2D
//

#include "Helpers.h"

#include <Vortex2D/Renderer/Texture.h>
#include <Vortex2D/Renderer/Disable.h>
#include <Vortex2D/Renderer/Reader.h>
#include <Vortex2D/Renderer/Writer.h>

#include <Vortex2D/Engine/LinearSolver/ConjugateGradient.h>
#include <Vortex2D/Engine/LinearSolver/SuccessiveOverRelaxation.h>
#include <Vortex2D/Engine/LinearSolver/Multigrid.h>
#include <Vortex2D/Engine/LinearSolver/Transfer.h>
#include <Vortex2D/Engine/Pressure.h>

#include <algorithm>
#include <chrono>
#include <numeric>

using namespace Vortex2D::Renderer;
using namespace Vortex2D::Fluid;
using ::testing::NiceMock;

TEST(LinearSolverTests, ReduceSum)
{
    Disable d(GL_BLEND);

    ReduceSum reduce(glm::vec2(10, 15));

    Buffer input(glm::vec2(10, 15), 1);
    input.ClampToBorder();

    std::vector<float> bData(10*15);
    float n = 1.0f;
    std::generate(bData.begin(), bData.end(), [&n]{ return n++; });
    Writer(input).Write(bData);

    Buffer output(glm::vec2(1), 1);
    output = reduce(input);

    float total = Reader(output).Read().GetFloat(0, 0);

    ASSERT_EQ(0.5f*150.0f*151.0f, total);
}

TEST(LinearSolverTests, ReduceInnerProduct)
{
    Disable d(GL_BLEND);

    ReduceSum reduce(glm::vec2(10, 15));

    Buffer input1(glm::vec2(10, 15), 1);
    Buffer input2(glm::vec2(10, 15), 1);

    std::vector<float> input1Data(10*15);
    float n = 1.0f;
    std::generate(input1Data.begin(), input1Data.end(), [&n]{ return n++; });
    Writer(input1).Write(input1Data);

    std::vector<float> input2Data(10*15, 2.0f);
    Writer(input2).Write(input2Data);

    Buffer output(glm::vec2(1), 1);
    output = reduce(input1, input2);

    float total = Reader(output).Read().GetFloat(0, 0);

    ASSERT_EQ(150.0f*151.0f, total);
}

TEST(LinearSolverTests, ReduceMax)
{
    Disable d(GL_BLEND);

    ReduceMax reduce(glm::vec2(10, 15));

    Buffer input(glm::vec2(10, 15), 1);

    std::vector<float> bData(10*15);
    float n = -1.0f;
    std::generate(bData.begin(), bData.end(), [&n]{ return n--; });
    Writer(input).Write(bData);

    Buffer output(glm::vec2(1), 1);
    output = reduce(input);

    float total = Reader(output).Read().GetFloat(0, 0);

    ASSERT_EQ(150.0f, total);
}

TEST(LinearSolverTests, Transfer_Prolongate)
{
    Disable d(GL_BLEND);

    glm::vec2 size(2);

    Transfer t;

    Buffer input(size, 1), output(glm::vec2(2) * size, 1);

    std::vector<float> data(size.x * size.y, 0.0f);
    std::iota(data.begin(), data.end(), 1.0f);
    Writer(input).Write(data);

    output = t.Prolongate(input);

    float total;

    total = (9*1 + 3*2 + 3*3 + 1*4) / 16.0f;
    EXPECT_FLOAT_EQ(total, Reader(output).Read().GetFloat(1, 1));

    total = (9*2 + 3*1 + 3*4 + 1*3) / 16.0f;
    EXPECT_FLOAT_EQ(total, Reader(output).Read().GetFloat(2, 1));

    total = (9*3 + 3*1 + 3*4 + 1*2) / 16.0f;
    EXPECT_FLOAT_EQ(total, Reader(output).Read().GetFloat(1, 2));

    total = (9*4 + 3*2 + 3*3 + 1*1) / 16.0f;
    EXPECT_FLOAT_EQ(total, Reader(output).Read().GetFloat(2, 2));
}

TEST(LinearSolverTests, Transfer_Restrict)
{
    Disable d(GL_BLEND);

    glm::vec2 size(3);

    Transfer t;

    Buffer input(glm::vec2(2) * size, 1), output(size, 1);

    std::vector<float> data(size.x * size.y * 4, 1.0f);
    std::iota(data.begin(), data.end(), 1.0f);
    Writer(input).Write(data);

    output = t.Restrict(input);

    float total = (1*8 + 3*9 + 3*10 + 1*11 +
            3*14 + 9*15 + 9*16 + 3*17 +
            3*20 + 9*21 + 9*22 + 3*23 +
            1*26 + 3*27 + 3*28 + 1*29) / 64.0f;

    EXPECT_FLOAT_EQ(total, Reader(output).Read().GetFloat(1, 1));
}

TEST(LinearSolverTests, Transfer_Symmetric)
{
    Disable d(GL_BLEND);

    glm::vec2 size(8);

    Transfer t;

    Buffer input(size, 1), output(glm::vec2(2) * size, 1);

    std::vector<float> data(size.x * size.y * 4, 1.0f);
    Writer(input).Write(data);

    output = t.Prolongate(input);
    input = t.Restrict(output);

    Reader reader(input);
    reader.Read();

    for (int i = 1; i < size.x - 1; i++)
    {
        for (int j = 1; j < size.y - 1; j++)
        {
            EXPECT_FLOAT_EQ(1.0f, reader.GetFloat(i, j));
        }
    }
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
            uint8_t value = 1 - dataData[i + j * size.x];
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
    SuccessiveOverRelaxation solver(size);

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
    SuccessiveOverRelaxation solver(size);

    solver.Init(data);
    solver.Solve(data, params);

    CheckPressure(size, sim.pressure, data, 1e-4f);

    std::cout << "Solved with number of iterations: " << params.OutIterations << std::endl;
}

TEST(LinearSolverTests, Simple_CG)
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
    sim.project(0.01f);

    LinearSolver::Data data(size);
    BuildLinearEquation(size, data, sim);

    LinearSolver::Parameters params(1000, 1e-5f);
    ConjugateGradient solver(size);
    solver.Init(data);
    solver.Solve(data, params);

    Reader reader(data.Pressure);
    reader.Read();

    CheckPressure(size, sim.pressure, data, params.ErrorTolerance);

    std::cout << "Solved with number of iterations: " << params.OutIterations << std::endl;
}

TEST(LinearSolverTests, Complex_PCG)
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

    LinearSolver::Parameters params(1000, 1e-5f);
    ConjugateGradient solver(size);
    solver.Init(data);
    solver.Solve(data, params);

    Reader reader(data.Pressure);
    reader.Read();

    CheckPressure(size, sim.pressure, data, params.ErrorTolerance);

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

    glm::vec2 size(32);

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
        SuccessiveOverRelaxation solver(size);

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
    sim.set_boundary(complex_boundary_phi);

    AddParticles(size, sim, complex_boundary_phi);

    sim.add_force(0.01f);
    sim.project(0.01f);

    LinearSolver::Data data(size);
    BuildLinearEquation(size, data, sim);

    LinearSolver::Parameters params(1000, 1e-5f);
    ConjugateGradient solver(size);

    solver.Init(data);

    auto start = std::chrono::system_clock::now();

    solver.Solve(data, params);

    auto end = std::chrono::system_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    std::cout << "Solved in time: " << elapsed.count() << std::endl;
    std::cout << "Solved with number of iterations: " << params.OutIterations << std::endl;
}
