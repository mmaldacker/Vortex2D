//
//  RigidbodyTests.cpp
//  Vortex2D
//

#include "VariationalHelpers.h"
#include "Verify.h"
#include "box2dgeometry.h"
#include "rigidbody.h"

#include <Vortex2D/Renderer/RenderTexture.h>
#include <Vortex2D/Engine/Pressure.h>
#include <Vortex2D/Engine/Rigidbody.h>
#include <Vortex2D/Engine/Boundaries.h>

using namespace Vortex2D::Renderer;
using namespace Vortex2D::Fluid;

extern Device* device;

TEST(RgidibodyTests, Div)
{
    glm::ivec2 size(20);

    FluidSim sim;
    sim.initialize(1.0f, size.x, size.y);
    sim.set_boundary(boundary_phi);

    AddParticles(size, sim, boundary_phi);

    sim.rigidgeom = new Box2DGeometry(0.3f, 0.2f);
    sim.rbd = new ::RigidBody(0.4f, *sim.rigidgeom);
    sim.rbd->setCOM(Vec2f(0.5f, 0.5f));
    sim.rbd->setAngle(0.0);
    sim.rbd->setAngularMomentum(0.5f);
    sim.rbd->setLinearVelocity(Vec2f(1.0f, 0.0f));

    sim.update_rigid_body_grids();

    sim.add_force(0.01f);

    Texture velocity(*device, size.x, size.y, vk::Format::eR32G32Sfloat);
    RenderTexture solidPhi(*device, size.x, size.y, vk::Format::eR32Sfloat);
    Texture liquidPhi(*device, size.x, size.y, vk::Format::eR32Sfloat);

    BuildInputs(*device, size, sim, velocity, solidPhi, liquidPhi);

    LinearSolver::Data data(*device, size, VMA_MEMORY_USAGE_CPU_ONLY);

    Buffer<glm::ivec2> valid(*device, size.x*size.y, VMA_MEMORY_USAGE_CPU_ONLY);

    Pressure pressure(*device, 0.01f, size, data, velocity, solidPhi, liquidPhi, valid);

    Vortex2D::Fluid::Rectangle rectangle(*device, {15.0f, 10.0f});
    Vortex2D::Fluid::RigidBody rigidBody(*device, Dimensions(size, 1.0f), rectangle, {0.0f, 0.0f});

    rigidBody.RecordPhi(solidPhi).Submit();
    rigidBody.RecordLocalPhi().Submit();

    float w;
    sim.rbd->getAngularVelocity(w);
    rigidBody.SetVelocities({1.0f, 0.0f}, w);

    rigidBody.BindDiv(data.B, data.Diagonal, liquidPhi);

    pressure.BuildLinearEquation();
    rigidBody.Div();
    device->Handle().waitIdle();

    PrintDiv(size, sim);
    PrintBuffer<float>(size, data.B);
    CheckDiv(size, data.B, sim);
}
