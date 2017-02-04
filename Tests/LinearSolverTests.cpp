//
//  LinearSolverTests.cpp
//  Vortex2D
//

#include <algorithm>
#include "gtest/gtest.h"
#include "Common.h"
#include "Texture.h"
#include "Disable.h"
#include "Reader.h"
#include "Writer.h"
#include "ConjugateGradient.h"
#include "SuccessiveOverRelaxation.h"
#include "variationalplusgfm/fluidsim.h"

using namespace Vortex2D::Renderer;
using namespace Vortex2D::Fluid;

TEST(LinearSolverTests, Reduce)
{
    Disable d(GL_BLEND);

    Reduce reduce(glm::vec2(10, 15));

    Buffer a(glm::vec2(10, 15), 1);
    Buffer b(glm::vec2(10, 15), 1);

    std::vector<float> aData(10*15, 1.0f);
    Writer(a).Write(aData);

    std::vector<float> bData(10*15);
    float n = 1.0f;
    std::generate(bData.begin(), bData.end(), [&n]{ return n++; });
    Writer(b).Write(bData);

    Buffer buffer(glm::vec2(1), 1);
    buffer = reduce(a, b);

    float total = Reader(buffer).Read().GetFloat(0, 0);

    ASSERT_EQ(0.5f*150.0f*151.0f, total);
}

class EmptyLinearSolver : public LinearSolver
{
public:
    void Init(Data& data)
    {

    }

    void Solve(Data& data)
    {

    }
};

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

//Boundary definition - several circles in a circular domain.

namespace
{

Vec2f c0(0.5f,0.5f), c1(0.7f,0.5f), c2(0.3f,0.35f), c3(0.5f,0.7f);
float rad0 = 0.4f,  rad1 = 0.1f,  rad2 = 0.1f,   rad3 = 0.1f;

}

float circle_phi(const Vec2f& position, const Vec2f& centre, float radius)
{
    return (dist(position,centre) - radius);
}

float boundary_phi(const Vec2f& position)
{
    float phi0 = -circle_phi(position, c0, rad0);

    return phi0;
}

float complex_boundary_phi(const Vec2f& position)
{
    float phi0 = -circle_phi(position, c0, rad0);
    float phi1 = circle_phi(position, c1, rad1);
    float phi2 = circle_phi(position, c2, rad2);
    float phi3 = circle_phi(position, c3, rad3);

    return min(min(phi0,phi1),min(phi2,phi3));
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

void AddParticles(const glm::vec2& size, FluidSim& sim)
{
    for(int i = 0; i < 4*sqr(size.x); ++i)
    {
        float x = randhashf(i*2, 0,1);
        float y = randhashf(i*2+1, 0,1);
        Vec2f pt(x,y);
        if (boundary_phi(pt) > 0 && pt[0] > 0.5)
        {
            sim.add_particle(pt);
        }
    }
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

    SuccessiveOverRelaxation solver(size, 200);

    solver.Init(data);
    solver.Solve(data);

    CheckPressure(size, sim.pressure, data, 1e-5);
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

    SuccessiveOverRelaxation solver(size, 500);

    solver.Init(data);
    solver.Solve(data);

    CheckPressure(size, sim.pressure, data, 1e-5);
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

    ConjugateGradient solver(size, 1000);
    solver.Init(data);
    solver.NormalSolve(data);

    Reader reader(data.Pressure);
    reader.Read();

    CheckPressure(size, sim.pressure, data, 1e-5);
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

    ConjugateGradient solver(size, 100);
    solver.Init(data);
    solver.Solve(data);

    Reader reader(data.Pressure);
    reader.Read();

    CheckPressure(size, sim.pressure, data, 1e-5);
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

    ConjugateGradient solver(size, 100);
    solver.Init(data);
    solver.Solve(data);

    Reader reader(data.Pressure);
    reader.Read();

    CheckPressure(size, sim.pressure, data, 1e-5);
}
