//
//  PressureTests.cpp
//  Vortex2D
//

#include "gtest/gtest.h"
#include "Helpers.h"
#include "Pressure.h"
#include "Writer.h"

using namespace Vortex2D::Renderer;
using namespace Vortex2D::Fluid;
using ::testing::NiceMock;
using ::testing::InvokeWithoutArgs;
using ::testing::_;

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
}

void CheckDiagonal(const glm::vec2& size, Buffer& buffer, FluidSim& sim)
{
    Reader reader(buffer);
    reader.Read();

    for (std::size_t i = 1; i < size.x - 1; i++)
    {
        for (std::size_t j = 1; j < size.y - 1; j++)
        {
            std::size_t index = i + size.x * j;
            EXPECT_FLOAT_EQ(sim.matrix(index, index), reader.GetFloat(i, j));
        }
    }
}

void CheckWeights(const glm::vec2& size, Buffer& buffer, FluidSim& sim)
{
    Reader reader(buffer);
    reader.Read();

    for (std::size_t i = 1; i < size.x - 1; i++)
    {
        for (std::size_t j = 1; j < size.y - 1; j++)
        {
            std::size_t index = i + size.x * j;
            EXPECT_FLOAT_EQ(sim.matrix(index + 1, index), reader.GetVec4(i, j).x);
            EXPECT_FLOAT_EQ(sim.matrix(index - 1, index), reader.GetVec4(i, j).y);
            EXPECT_FLOAT_EQ(sim.matrix(index, index + size.x), reader.GetVec4(i, j).z);
            EXPECT_FLOAT_EQ(sim.matrix(index, index - size.x), reader.GetVec4(i, j).w);
        }
    }
}

void CheckDiv(const glm::vec2& size, Buffer& buffer, FluidSim& sim)
{
    Reader reader(buffer);
    reader.Read();

    for (std::size_t i = 1; i < size.x - 1; i++)
    {
        for (std::size_t j = 1; j < size.y - 1; j++)
        {
            std::size_t index = i + size.x * j;
            EXPECT_FLOAT_EQ(sim.rhs[index], reader.GetVec2(i, j).y);
        }
    }
}

void CheckVelocity(const glm::vec2& size, Buffer& buffer, FluidSim& sim)
{
    Reader reader(buffer);
    reader.Read();

    for (std::size_t i = 1; i < size.x - 1; i++)
    {
        for (std::size_t j = 1; j < size.y - 1; j++)
        {
            std::size_t index = i + size.x * j;
            ASSERT_NEAR(sim.u(i, j), reader.GetVec2(i, j).x, 1e-6) << "Mismatch at " << i << "," << j;
            ASSERT_NEAR(sim.v(i, j), reader.GetVec2(i, j).y, 1e-6) << "Mismatch at " << i << "," << j;
        }
    }
}

void SetForce(Buffer& buffer)
{
    std::vector<glm::vec2> force(buffer.Width() * buffer.Height(), glm::vec2(0.0f, -0.1f));
    Writer(buffer).Write(force);
}

void SetSolidPhi(Buffer& buffer, FluidSim& sim)
{
    std::vector<float> phi(buffer.Width() * buffer.Height(), 0.0f);
    for (int i = 0; i < buffer.Width(); i++)
    {
        for (int j = 0; j < buffer.Height(); j++)
        {
            phi[i + j * buffer.Width()] = sim.nodal_solid_phi(i/2, j/2);
        }
    }

    Writer(buffer).Write(phi);
}

void SetLiquidPhi(Buffer& buffer, FluidSim& sim)
{
    std::vector<float> phi(buffer.Width() * buffer.Height(), 0.0f);
    for (int i = 0; i < buffer.Width(); i++)
    {
        for (int j = 0; j < buffer.Height(); j++)
        {
            phi[i + j * buffer.Width()] = sim.liquid_phi(i, j);
        }
    }

    Writer(buffer).Write(phi);
}

TEST(PressureTest, LinearEquationSetup_Simple)
{
    Disable d(GL_BLEND);

    glm::vec2 size(10);

    FluidSim sim;
    sim.initialize(1.0f, size.x, size.y);
    sim.set_boundary(boundary_phi);

    AddParticles(size, sim, boundary_phi);

    sim.add_force(0.01f);
    sim.project(0.01f);

    LinearSolver::Data data(size);
    NiceMock<MockLinearSolver> solver;

    Buffer velocity(size, 2, true);
    SetForce(velocity);

    Buffer solidPhi(glm::vec2(2)*size, 1);
    solidPhi.ClampToEdge();
    SetSolidPhi(solidPhi, sim);

    Buffer liquidPhi(size, 1);
    liquidPhi.ClampToEdge();
    SetLiquidPhi(liquidPhi, sim);

    Buffer solidVelocity(size, 2);
    // leave empty

    Pressure pressure(0.01f, size, solver, data, velocity, solidPhi, liquidPhi, solidVelocity);

    LinearSolver::Parameters params(0);
    pressure.Solve(params);

    CheckWeights(size, data.Weights, sim);
    CheckDiv(size, data.Pressure, sim);
    CheckDiagonal(size, data.Diagonal, sim);
}

TEST(PressureTest, LinearEquationSetup_Complex)
{
    Disable d(GL_BLEND);

    glm::vec2 size(10);

    FluidSim sim;
    sim.initialize(1.0f, size.x, size.y);
    sim.set_boundary(boundary_phi);

    AddParticles(size, sim, complex_boundary_phi);

    sim.add_force(0.01f);
    sim.project(0.01f);

    LinearSolver::Data data(size);
    NiceMock<MockLinearSolver> solver;

    Buffer velocity(size, 2, true);
    SetForce(velocity);

    Buffer solidPhi(glm::vec2(2)*size, 1);
    solidPhi.ClampToEdge();
    SetSolidPhi(solidPhi, sim);

    Buffer liquidPhi(size, 1);
    liquidPhi.ClampToEdge();
    SetLiquidPhi(liquidPhi, sim);

    Buffer solidVelocity(size, 2);
    // leave empty

    Pressure pressure(0.01f, size, solver, data, velocity, solidPhi, liquidPhi, solidVelocity);

    LinearSolver::Parameters params(0);
    pressure.Solve(params);

    CheckWeights(size, data.Weights, sim);
    CheckDiv(size, data.Pressure, sim);
    CheckDiagonal(size, data.Diagonal, sim);
}

TEST(PressureTest, Project_Simple)
{
    Disable d(GL_BLEND);

    glm::vec2 size(10);

    FluidSim sim;
    sim.initialize(1.0f, size.x, size.y);
    sim.set_boundary(boundary_phi);

    AddParticles(size, sim, boundary_phi);

    sim.add_force(0.01f);
    sim.project(0.01f);

    LinearSolver::Data data(size);
    MockLinearSolver solver;

    Buffer velocity(size, 2, true);
    SetForce(velocity);

    Buffer solidPhi(glm::vec2(2)*size, 1);
    solidPhi.ClampToEdge();
    SetSolidPhi(solidPhi, sim);

    Buffer liquidPhi(size, 1);
    liquidPhi.ClampToEdge();
    SetLiquidPhi(liquidPhi, sim);

    Buffer solidVelocity(size, 2);
    // leave empty

    Pressure pressure(0.01f, size, solver, data, velocity, solidPhi, liquidPhi, solidVelocity);

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

    CheckVelocity(size, velocity, sim);
}

TEST(PressureTest, Project_Complex)
{
    Disable d(GL_BLEND);

    glm::vec2 size(10);

    FluidSim sim;
    sim.initialize(1.0f, size.x, size.y);
    sim.set_boundary(boundary_phi);

    AddParticles(size, sim, complex_boundary_phi);

    sim.add_force(0.01f);
    sim.project(0.01f);

    LinearSolver::Data data(size);
    MockLinearSolver solver;

    Buffer velocity(size, 2, true);
    SetForce(velocity);

    Buffer solidPhi(glm::vec2(2)*size, 1);
    solidPhi.ClampToEdge();
    SetSolidPhi(solidPhi, sim);

    Buffer liquidPhi(size, 1);
    liquidPhi.ClampToEdge();
    SetLiquidPhi(liquidPhi, sim);

    Buffer solidVelocity(size, 2);
    // leave empty

    Pressure pressure(0.01f, size, solver, data, velocity, solidPhi, liquidPhi, solidVelocity);

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

    CheckVelocity(size, velocity, sim);
}
