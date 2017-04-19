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

TEST(AdvectionTests, Advect)
{
    Disable d(GL_BLEND);

    glm::vec2 size(10);

    Buffer velocity(size, 2, true);

    std::vector<glm::vec2> velocityData(size.x * size.y, glm::vec2(0.0f));
    velocityData[3 + size.x * 4] = glm::vec2(1.0f, 0.0f);
    Writer(velocity).Write(velocityData);

    Reader(velocity).Read().Print();

    Buffer buffer(size, 1, true);

    std::vector<float> bufferData(size.x * size.y, 0.0f);
    bufferData[3 + size.x * 4] = 1.0f;
    Writer(buffer).Write(bufferData);

    Advection advection(1.0f, velocity);
    advection.Advect(buffer);

    Reader(buffer).Read().Print();

    // FIXME assert on something
}
