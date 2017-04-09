//
//  PressureTests.cpp
//  Vortex2D
//

#include "Helpers.h"

#include <Vortex2D/Renderer/Disable.h>

#include <Vortex2D/Engine/Pressure.h>

using namespace Vortex2D::Renderer;
using namespace Vortex2D::Fluid;
using ::testing::NiceMock;
using ::testing::InvokeWithoutArgs;
using ::testing::_;
using ::testing::Invoke;

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
    Reader reader(buffer);
    reader.Read();

    for (std::size_t i = 1; i < size.x - 1; i++)
    {
        for (std::size_t j = 1; j < size.y - 1; j++)
        {
            std::size_t index = i + size.x * j;
            EXPECT_NEAR(sim.matrix(index, index), reader.GetFloat(i, j), error);
        }
    }
}

void CheckWeights(const glm::vec2& size, Buffer& buffer, FluidSim& sim, float error = 1e-6)
{
    Reader reader(buffer);
    reader.Read();

    for (std::size_t i = 1; i < size.x - 1; i++)
    {
        for (std::size_t j = 1; j < size.y - 1; j++)
        {
            std::size_t index = i + size.x * j;
            EXPECT_NEAR(sim.matrix(index + 1, index), reader.GetVec4(i, j).x, error);
            EXPECT_NEAR(sim.matrix(index - 1, index), reader.GetVec4(i, j).y, error);
            EXPECT_NEAR(sim.matrix(index, index + size.x), reader.GetVec4(i, j).z, error);
            EXPECT_NEAR(sim.matrix(index, index - size.x), reader.GetVec4(i, j).w, error);
        }
    }
}

void CheckDiv(const glm::vec2& size, Buffer& buffer, FluidSim& sim, float error = 1e-6)
{
    Reader reader(buffer);
    reader.Read();

    for (std::size_t i = 1; i < size.x - 1; i++)
    {
        for (std::size_t j = 1; j < size.y - 1; j++)
        {
            std::size_t index = i + size.x * j;
            EXPECT_NEAR(sim.rhs[index], reader.GetVec2(i, j).y, error);
        }
    }
}

TEST(PressureTest, LinearEquationSetup_Simple)
{
    Disable d(GL_BLEND);

    glm::vec2 size(50);

    FluidSim sim;
    sim.initialize(1.0f, size.x, size.y);
    sim.set_boundary(boundary_phi);

    AddParticles(size, sim, boundary_phi);

    sim.add_force(0.01f);

    LinearSolver::Data data(size);
    NiceMock<MockLinearSolver> solver;

    Buffer velocity(size, 2, true);
    SetVelocity(velocity, sim);

    sim.project(0.01f);

    Buffer solidPhi(glm::vec2(2)*size, 1);
    SetSolidPhi(solidPhi, sim);

    Buffer liquidPhi(size, 1);
    SetLiquidPhi(liquidPhi, sim);

    Buffer solidVelocity(size, 2);
    // leave empty

    Pressure pressure(0.01f, size, solver, data, velocity, solidPhi, liquidPhi, solidVelocity);

    LinearSolver::Parameters params(0);
    pressure.Solve(params);

    CheckWeights(size, data.Weights, sim, 1e-3); // FIXME can we reduce error tolerance?
    CheckDiv(size, data.Pressure, sim);
    CheckDiagonal(size, data.Diagonal, sim, 1e-3); // FIXME can we reduce error tolerance?
}

TEST(PressureTest, LinearEquationSetup_Complex)
{
    Disable d(GL_BLEND);

    glm::vec2 size(50);

    FluidSim sim;
    sim.initialize(1.0f, size.x, size.y);
    sim.set_boundary(boundary_phi);

    AddParticles(size, sim, complex_boundary_phi);

    sim.add_force(0.01f);

    LinearSolver::Data data(size);
    NiceMock<MockLinearSolver> solver;

    Buffer velocity(size, 2, true);
    SetVelocity(velocity, sim);

    sim.project(0.01f);

    Buffer solidPhi(glm::vec2(2)*size, 1);
    SetSolidPhi(solidPhi, sim);

    Buffer liquidPhi(size, 1);
    SetLiquidPhi(liquidPhi, sim);

    Buffer solidVelocity(size, 2);
    // leave empty

    Pressure pressure(0.01f, size, solver, data, velocity, solidPhi, liquidPhi, solidVelocity);

    LinearSolver::Parameters params(0);
    pressure.Solve(params);

    CheckWeights(size, data.Weights, sim, 1e-3); // FIXME can we reduce error tolerance?
    CheckDiv(size, data.Pressure, sim);
    CheckDiagonal(size, data.Diagonal, sim, 1e-3); // FIXME can we reduce error tolerance?
}

TEST(PressureTest, Project_Simple)
{
    Disable d(GL_BLEND);

    glm::vec2 size(50);

    FluidSim sim;
    sim.initialize(1.0f, size.x, size.y);
    sim.set_boundary(boundary_phi);

    AddParticles(size, sim, boundary_phi);

    sim.add_force(0.01f);

    LinearSolver::Data data(size);
    MockLinearSolver solver;

    Buffer velocity(size, 2, true);
    SetVelocity(velocity, sim);

    sim.project(0.01f);

    Buffer solidPhi(glm::vec2(2)*size, 1);
    SetSolidPhi(solidPhi, sim);

    Buffer liquidPhi(size, 1);
    SetLiquidPhi(liquidPhi, sim);

    Buffer solidVelocity(size, 2);
    // leave empty

    Pressure pressure(0.01f, size, solver, data, velocity, solidPhi, liquidPhi, solidVelocity);

    EXPECT_CALL(solver, Build(_, _, _, _, _));
    EXPECT_CALL(solver, Init(_));
    EXPECT_CALL(solver, Solve(_, _)).WillOnce(InvokeWithoutArgs([&]
    {
        std::vector<glm::vec2> pressureData(size.x*size.y, glm::vec2(0.0f, 0.0f));
        for (std::size_t i = 0; i < pressureData.size(); i++)
        {
            pressureData[i].x = sim.pressure[i];
        }

        Writer(data.Pressure).Write(pressureData);
    }));

    LinearSolver::Parameters params(0);
    pressure.Solve(params);

    CheckVelocity(velocity, sim);
}

TEST(PressureTest, Project_Complex)
{
    Disable d(GL_BLEND);

    glm::vec2 size(50);

    FluidSim sim;
    sim.initialize(1.0f, size.x, size.y);
    sim.set_boundary(boundary_phi);

    AddParticles(size, sim, complex_boundary_phi);

    sim.add_force(0.01f);

    LinearSolver::Data data(size);
    MockLinearSolver solver;

    Buffer velocity(size, 2, true);
    SetVelocity(velocity, sim);

    sim.project(0.01f);

    Buffer solidPhi(glm::vec2(2)*size, 1);
    SetSolidPhi(solidPhi, sim);

    Buffer liquidPhi(size, 1);
    SetLiquidPhi(liquidPhi, sim);

    Buffer solidVelocity(size, 2);
    // leave empty

    Pressure pressure(0.01f, size, solver, data, velocity, solidPhi, liquidPhi, solidVelocity);

    EXPECT_CALL(solver, Build(_, _, _, _, _));
    EXPECT_CALL(solver, Init(_));
    EXPECT_CALL(solver, Solve(_, _)).WillOnce(InvokeWithoutArgs([&]
    {
        std::vector<glm::vec2> pressureData(size.x*size.y, glm::vec2(0.0f, 0.0f));
        for (std::size_t i = 0; i < pressureData.size(); i++)
        {
            pressureData[i].x = sim.pressure[i];
        }

        Writer(data.Pressure).Write(pressureData);
    }));

    LinearSolver::Parameters params(0);
    pressure.Solve(params);

    CheckVelocity(velocity, sim);
}
