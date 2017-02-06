//
//  ExtrapolateTests.cpp
//  Vortex2D
//

#include "Helpers.h"
#include "Extrapolation.h"

using namespace Vortex2D::Fluid;
using namespace Vortex2D::Renderer;

TEST(ExtrapolateTest, Extrapolate)
{
    Disable d(GL_BLEND);

    glm::vec2 size(50);

    FluidSim sim;
    sim.initialize(1.0f, size.x, size.y);
    sim.set_boundary(boundary_phi);

    AddParticles(size, sim, complex_boundary_phi);

    sim.add_force(0.01f);
    sim.project(0.01f);

    // FluidSim ignores the borders which we don't, so need to make them match
    for (int i = 0; i < size.x; i++)
    {
        sim.v(i, 0) = 0.0;
    }

    Buffer velocity(size, 2, true);
    SetVelocity(velocity, sim);

    Buffer solidPhi(glm::vec2(2)*size, 1);
    solidPhi.ClampToEdge();
    SetSolidPhi(solidPhi, sim);

    extrapolate(sim.u, sim.u_valid);
    extrapolate(sim.v, sim.v_valid);

    Extrapolation extrapolation(size, velocity, solidPhi);
    extrapolation.Extrapolate();

    CheckVelocity(size, velocity, sim);
}

TEST(ExtrapolateTest, Constrain)
{
    Disable d(GL_BLEND);

    glm::vec2 size(10);

    FluidSim sim;
    sim.initialize(1.0f, size.x, size.y);
    sim.set_boundary(boundary_phi);

    AddParticles(size, sim, complex_boundary_phi);

    sim.add_force(0.01f);
    sim.project(0.01f);

    // FluidSim ignores the borders which we don't, so need to make them match
    for (int i = 0; i < size.x; i++)
    {
        sim.v(i, 0) = 0.0;
    }

    Buffer velocity(size, 2, true);
    SetVelocity(velocity, sim);

    Buffer solidPhi(glm::vec2(2)*size, 1);
    solidPhi.ClampToEdge();
    SetSolidPhi(solidPhi, sim);

    extrapolate(sim.u, sim.u_valid);
    extrapolate(sim.v, sim.v_valid);
    sim.constrain_velocity();

    Extrapolation extrapolation(size, velocity, solidPhi);
    extrapolation.Extrapolate();

    Reader(velocity).Read().Print();

    extrapolation.ConstrainVelocity();

    PrintVelocity(size, sim);
    Reader(velocity).Read().Print();

    // FIXME not working yet
    //CheckVelocity(size, velocity, sim);
}
