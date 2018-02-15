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

void PrintRigidBody(const glm::ivec2& size, FluidSim& sim)
{
    for (int i = 0; i < size.x; i++)
    {
        for (int j = 0; j < size.y; j++)
        {
            Vec2f pos((i + 0.5f) / size.x, (j + 0.5) / size.x);
            std::cout << "(" << sim.rbd->getSignedDist(pos) * size.x << ")";
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;
}

TEST(RgidibodyTests, Div)
{
    glm::ivec2 size(20);

    FluidSim sim;
    sim.initialize(1.0f, size.x, size.y);
    sim.set_boundary(boundary_phi);

    AddParticles(size, sim, boundary_phi);

    // setup rigid body
    sim.rigidgeom = new Box2DGeometry(0.3f, 0.2f);
    sim.rbd = new ::RigidBody(0.4f, *sim.rigidgeom);
    sim.rbd->setCOM(Vec2f(0.5f, 0.5f));
    sim.rbd->setAngle(0.0);
    sim.rbd->setAngularMomentum(0.2f);
    sim.rbd->setLinearVelocity(Vec2f(0.1f, 0.0f));

    // get velocities
    float w; Vec2f v;
    sim.rbd->getAngularVelocity(w);
    sim.rbd->getLinearVelocity(v);

    sim.update_rigid_body_grids();

    sim.add_force(0.01f);

    Texture velocity(*device, size.x, size.y, vk::Format::eR32G32Sfloat);
    RenderTexture solidPhi(*device, size.x, size.y, vk::Format::eR32Sfloat);
    Texture liquidPhi(*device, size.x, size.y, vk::Format::eR32Sfloat);

    BuildInputs(*device, size, sim, velocity, solidPhi, liquidPhi);

    LinearSolver::Data data(*device, size, VMA_MEMORY_USAGE_CPU_ONLY);

    Buffer<glm::ivec2> valid(*device, size.x*size.y, VMA_MEMORY_USAGE_CPU_ONLY);

    Pressure pressure(*device, 0.01f, size, data, velocity, solidPhi, liquidPhi, valid);

    Vortex2D::Fluid::Rectangle rectangle(*device, {6.0f, 4.0f});
    rectangle.Anchor = {3.0f, 2.0f};

    Vortex2D::Fluid::RigidBody rigidBody(*device, Dimensions(size, 1.0f), rectangle, {0.0f, 0.0f});

    rigidBody.Position = {10.0f, 10.0f};
    rigidBody.UpdatePosition();

    solidPhi.View = rigidBody.View();
    rigidBody.RecordPhi(solidPhi).Submit();
    rigidBody.RecordLocalPhi().Submit();

    rigidBody.SetVelocities(glm::vec2(v[0], v[1]) * glm::vec2(size.x), w);

    rigidBody.BindDiv(data.B, data.Diagonal, liquidPhi);

    pressure.BuildLinearEquation();
    rigidBody.Div();
    device->Handle().waitIdle();

    PrintDiv(size, sim);
    PrintBuffer<float>(size, data.B);
    CheckDiv(size, data.B, sim, 1e-2); // TODO improve accuracy
}

TEST(RgidibodyTests, ReduceJSum)
{
    glm::ivec2 size(10, 15);
    int n = size.x * size.y;
    Buffer<Vortex2D::Fluid::RigidBody::Velocity> input(*device, n, VMA_MEMORY_USAGE_CPU_ONLY);
    Buffer<Vortex2D::Fluid::RigidBody::Velocity> output(*device, 1, VMA_MEMORY_USAGE_CPU_ONLY);

    ReduceJ reduce(*device, size);
    auto reduceBound = reduce.Bind(input, output);

    std::vector<Vortex2D::Fluid::RigidBody::Velocity> inputData(n);

    {
        float n = 1.0f;
        std::generate(inputData.begin(), inputData.end(),
                      [&n]{ return Vortex2D::Fluid::RigidBody::Velocity{glm::vec2(1.0f, -2.0), n++}; });
    }

    CopyFrom(input, inputData);

    ExecuteCommand(*device, [&](vk::CommandBuffer commandBuffer)
    {
       reduceBound.Record(commandBuffer);
    });

    std::vector<Vortex2D::Fluid::RigidBody::Velocity> outputData(1);
    CopyTo(output, outputData);

    ASSERT_EQ(0.5f * n * (n + 1), outputData[0].angular_velocity);
    ASSERT_EQ(1.0f * n, outputData[0].velocity.x);
    ASSERT_EQ(-2.0f * n, outputData[0].velocity.y);
}

TEST(RgidibodyTests, Pressure)
{
    glm::ivec2 size(20);

    FluidSim sim;
    sim.initialize(1.0f, size.x, size.y);
    sim.set_boundary(boundary_phi);

    AddParticles(size, sim, boundary_phi);

    // setup rigid body
    sim.rigidgeom = new Box2DGeometry(0.3f, 0.2f);
    sim.rbd = new ::RigidBody(0.4f, *sim.rigidgeom);
    sim.rbd->setCOM(Vec2f(0.5f, 0.5f));
    sim.rbd->setAngle(0.0);
    sim.rbd->setAngularMomentum(0.2f);
    sim.rbd->setLinearVelocity(Vec2f(0.1f, 0.0f));

    // get velocities
    float w; Vec2f v;
    sim.rbd->getAngularVelocity(w);
    sim.rbd->getLinearVelocity(v);

    sim.update_rigid_body_grids();

    sim.add_force(0.01f);

    Texture velocity(*device, size.x, size.y, vk::Format::eR32G32Sfloat);
    RenderTexture solidPhi(*device, size.x, size.y, vk::Format::eR32Sfloat);
    Texture liquidPhi(*device, size.x, size.y, vk::Format::eR32Sfloat);

    BuildInputs(*device, size, sim, velocity, solidPhi, liquidPhi);

    LinearSolver::Data data(*device, size, VMA_MEMORY_USAGE_CPU_ONLY);

    std::vector<float> computedPressureData(size.x*size.y, 0.0f);
    for (std::size_t i = 0; i < computedPressureData.size(); i++)
    {
        computedPressureData[i] = (float)sim.pressure[i];
    }
    CopyFrom(data.X, computedPressureData);

    Buffer<glm::ivec2> valid(*device, size.x*size.y, VMA_MEMORY_USAGE_CPU_ONLY);

    Pressure pressure(*device, 0.01f, size, data, velocity, solidPhi, liquidPhi, valid);

    Vortex2D::Fluid::Rectangle rectangle(*device, {6.0f, 4.0f});
    rectangle.Anchor = {3.0f, 2.0f};

    Vortex2D::Fluid::RigidBody rigidBody(*device, Dimensions(size, 1.0f), rectangle, {0.0f, 0.0f});

    rigidBody.Position = {10.0f, 10.0f};
    rigidBody.UpdatePosition();

    solidPhi.View = rigidBody.View();
    rigidBody.RecordPhi(solidPhi).Submit();
    rigidBody.RecordLocalPhi().Submit();

    rigidBody.SetVelocities(glm::vec2(v[0], v[1]) * glm::vec2(size.x), w);

    Buffer<Vortex2D::Fluid::RigidBody::Velocity> force(*device, size.x*size.y, VMA_MEMORY_USAGE_CPU_ONLY);

    rigidBody.BindPressure(liquidPhi, data.X, force);

    pressure.ApplyPressure();
    rigidBody.Pressure();

    device->Handle().waitIdle();

    auto rigidbodyForce = rigidBody.GetVelocities();

    std::cout << rigidbodyForce.velocity.x << ","
              << rigidbodyForce.velocity.y << ","
              << rigidbodyForce.angular_velocity << std::endl;
    FAIL();
}
