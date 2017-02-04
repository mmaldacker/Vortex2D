//
//  PressureTests.cpp
//  Vortex2D
//

#include "gtest/gtest.h"
#include "Helpers.h"
#include "Pressure.h"

using namespace Vortex2D::Renderer;
using namespace Vortex2D::Fluid;

TEST(PressureTest, Div)
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
    EmptyLinearSolver solver;

    Buffer velocity(size, 2, true);
    Buffer solidPhi(size, 1);
    Buffer liquidPhi(size, 1);
    Buffer solidVelocity(size, 2);

    Pressure pressure(0.01f, solver, data, velocity, solidPhi, liquidPhi, solidVelocity);
}
