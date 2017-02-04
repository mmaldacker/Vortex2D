//
//  LinearSolverTests.cpp
//  Vortex2D
//

#include <algorithm>
#include "gtest/gtest.h"
#include "Helpers.h"
#include "Texture.h"
#include "Disable.h"
#include "Reader.h"
#include "Writer.h"
#include "ConjugateGradient.h"
#include "SuccessiveOverRelaxation.h"

using namespace Vortex2D::Renderer;
using namespace Vortex2D::Fluid;

TEST(LinearSolverTests, ReduceSum)
{
    Disable d(GL_BLEND);

    ReduceSum reduce(glm::vec2(10, 15));

    Buffer input(glm::vec2(10, 15), 1);

    std::vector<float> bData(10*15);
    float n = 1.0f;
    std::generate(bData.begin(), bData.end(), [&n]{ return n++; });
    Writer(input).Write(bData);

    Buffer output(glm::vec2(1), 1);
    output = reduce(input);

    float total = Reader(output).Read().GetFloat(0, 0);

    ASSERT_EQ(0.5f*150.0f*151.0f, total);
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

TEST(LinearSolverTests, RenderMask)
{
    Disable d(GL_BLEND);

    glm::vec2 size(10);

    Buffer buffer(size, 1, true, true);
    LinearSolver::Data data(size);

    std::vector<float> dataData(size.x * size.y, 0.0f);
    dataData[15] = 1.0f;
    Writer(data.Diagonal).Write(dataData);

    EmptyLinearSolver solver;
    solver.RenderMask(buffer, data);

    Reader reader(buffer);
    reader.Read();

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
            ASSERT_NEAR(value, reader.GetVec2(i, j).x, error);
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

    AddParticles(size, sim);

    sim.add_force(0.01f);
    sim.project(0.01f);

    LinearSolver::Data data(size);
    BuildLinearEquation(size, data, sim);

    LinearSolver::Parameters params(200);
    SuccessiveOverRelaxation solver(size);

    solver.Init(data);
    solver.Solve(data, params);

    CheckPressure(size, sim.pressure, data, 1e-5f);

    std::cout << "Solved with number of iterations: " << params.OutIterations << std::endl;
}

TEST(LinearSolverTests, Complex_SOR)
{
    Disable d(GL_BLEND);

    glm::vec2 size(50);

    FluidSim sim;
    sim.initialize(1.0f, size.x, size.y);
    sim.set_boundary(complex_boundary_phi);

    AddParticles(size, sim);

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

    AddParticles(size, sim);

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

    AddParticles(size, sim);

    sim.add_force(1.0f);
    sim.project(1.0f);

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

    AddParticles(size, sim);

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
