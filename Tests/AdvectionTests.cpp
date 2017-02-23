//
//  AdvectionTests.cpp
//  Vortex2D
//

#include "Helpers.h"

#include <Vortex2D/Renderer/Disable.h>

#include <Vortex2D/Engine/Advection.h>

using namespace Vortex2D::Renderer;
using namespace Vortex2D::Fluid;

TEST(AdvectionTests, AdvectVelocity_Simple)
{
    Disable d(GL_BLEND);

    glm::vec2 size(50);

    FluidSim sim;
    sim.initialize(1.0f, size.x, size.y);
    sim.set_boundary(boundary_phi);

    AddParticles(size, sim, boundary_phi);

    sim.add_force(0.01f);
    sim.advance(0.01f);

    Buffer velocity(size, 2, true);
    SetVelocity(velocity, sim);

    Advection advection(0.01f, velocity);
    advection.Advect();

    // using low error tolerance due to different accuracy algorithms used
    CheckVelocity(velocity, sim, 1e-3);
}

TEST(AdvectionTests, AdvectVelocity_Complex)
{
    Disable d(GL_BLEND);

    glm::vec2 size(50);

    FluidSim sim;
    sim.initialize(1.0f, size.x, size.y);
    sim.set_boundary(boundary_phi);

    AddParticles(size, sim, boundary_phi);

    sim.add_force(0.01f);
    sim.advance(0.01f);

    Buffer velocity(size, 2, true);
    SetVelocity(velocity, sim);

    Advection advection(0.01f, velocity);
    advection.Advect();

    // using low error tolerance due to different accuracy algorithms used
    CheckVelocity(velocity, sim, 1e-3);
}
