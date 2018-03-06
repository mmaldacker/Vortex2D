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
#include <Vortex2D/Engine/Extrapolation.h>

using namespace Vortex2D::Renderer;
using namespace Vortex2D::Fluid;

extern Device* device;

void PrintRigidBody(const glm::ivec2& size, FluidSim& sim)
{
    for (int i = 0; i < size.x; i++)
    {
        for (int j = 0; j < size.y; j++)
        {
            Vec2f pos((i + 0.5f) / size.x, (j + 0.5f) / size.x);
            std::cout << "(" << sim.rbd->getSignedDist(pos) * size.x << ")";
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;
}

void PrintForce(const glm::ivec2& size, Buffer<Vortex2D::Fluid::RigidBody::Velocity>& force)
{
    std::vector<Vortex2D::Fluid::RigidBody::Velocity> outForce(size.x*size.y);
    CopyTo(force, outForce);
    for (int i = 0; i < size.x; i++)
    {
        for (int j = 0; j < size.y; j++)
        {
            int index = i + j * size.x;
            std::cout << "(("
                      << outForce[index].velocity.x
                      << ","
                      << outForce[index].velocity.y
                      << "),"
                      << outForce[index].angular_velocity
                      << ")";
        }

        std::cout << std::endl;
    }

    std::cout << std::endl;
}

TEST(RigidbodyTests, Div)
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
    sim.rbd->setAngularMomentum(0.0f);
    sim.rbd->setLinearVelocity(Vec2f(0.0f, 0.0f));

    sim.update_rigid_body_grids();
    sim.add_force(0.01f);

    Velocity velocity(*device, size);
    RenderTexture solidPhi(*device, size.x, size.y, vk::Format::eR32Sfloat);
    Texture liquidPhi(*device, size.x, size.y, vk::Format::eR32Sfloat);

    BuildInputs(*device, size, sim, velocity, solidPhi, liquidPhi);
    SetSolidPhi(*device, size, solidPhi, sim, (float)size.x);

    LinearSolver::Data data(*device, size, VMA_MEMORY_USAGE_CPU_ONLY);
    Buffer<glm::ivec2> valid(*device, size.x*size.y, VMA_MEMORY_USAGE_CPU_ONLY);
    Pressure pressure(*device, 0.01f, size, data, velocity, solidPhi, liquidPhi, valid);

    Vortex2D::Fluid::Rectangle rectangle(*device, {6.0f, 4.0f});
    Vortex2D::Fluid::RigidBody rigidBody(*device,
                                         Dimensions(size, 1.0f),
                                         rectangle, {3.0f, 2.0f},
                                         solidPhi,
                                         Vortex2D::Fluid::RigidBody::Type::eStatic);

    rigidBody.Anchor = {3.0f, 2.0f};
    rigidBody.Position = {10.0f, 10.0f};
    rigidBody.UpdatePosition();

    rigidBody.RenderPhi();

    rigidBody.SetVelocities(glm::vec2(0.0f, 0.0f), 0.0f);
    rigidBody.BindDiv(data.B, data.Diagonal, liquidPhi);
    pressure.BuildLinearEquation();
    rigidBody.Div();
    device->Handle().waitIdle();

    CheckDiv(size, data.B, sim);
}

TEST(RigidbodyTests, VelocityDiv)
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
    sim.rbd->setAngularMomentum(0.0f);
    sim.rbd->setLinearVelocity(Vec2f(0.1f, 0.0f));

    // get velocities
    Vec2f v;
    sim.rbd->getLinearVelocity(v);

    sim.update_rigid_body_grids();
    sim.add_force(0.01f);

    Velocity velocity(*device, size);
    RenderTexture solidPhi(*device, size.x, size.y, vk::Format::eR32Sfloat);
    Texture liquidPhi(*device, size.x, size.y, vk::Format::eR32Sfloat);

    BuildInputs(*device, size, sim, velocity, solidPhi, liquidPhi);
    SetSolidPhi(*device, size, solidPhi, sim, (float)size.x);

    LinearSolver::Data data(*device, size, VMA_MEMORY_USAGE_CPU_ONLY);
    Buffer<glm::ivec2> valid(*device, size.x*size.y, VMA_MEMORY_USAGE_CPU_ONLY);
    Pressure pressure(*device, 0.01f, size, data, velocity, solidPhi, liquidPhi, valid);

    Vortex2D::Fluid::Rectangle rectangle(*device, {6.0f, 4.0f});
    Vortex2D::Fluid::RigidBody rigidBody(*device,
                                         Dimensions(size, 1.0f),
                                         rectangle, {0.0f, 0.0f},
                                         solidPhi,
                                         Vortex2D::Fluid::RigidBody::Type::eStatic);

    rigidBody.Anchor = {3.0f, 2.0f};
    rigidBody.Position = {10.0f, 10.0f};
    rigidBody.UpdatePosition();

    rigidBody.RenderPhi();

    rigidBody.SetVelocities(glm::vec2(v[0], v[1]) * glm::vec2((float)size.x), 0.0f);
    rigidBody.BindDiv(data.B, data.Diagonal, liquidPhi);
    pressure.BuildLinearEquation();
    rigidBody.Div();
    device->Handle().waitIdle();

    CheckDiv(size, data.B, sim);
}

TEST(RigidbodyTests, RotationDiv)
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
    sim.rbd->setLinearVelocity(Vec2f(0.0f, 0.0f));

    // get velocities
    float w; Vec2f v;
    sim.rbd->getAngularVelocity(w);
    sim.rbd->getLinearVelocity(v);

    sim.update_rigid_body_grids();
    sim.add_force(0.01f);

    Velocity velocity(*device, size);
    RenderTexture solidPhi(*device, size.x, size.y, vk::Format::eR32Sfloat);
    Texture liquidPhi(*device, size.x, size.y, vk::Format::eR32Sfloat);

    BuildInputs(*device, size, sim, velocity, solidPhi, liquidPhi);
    SetSolidPhi(*device, size, solidPhi, sim, (float)size.x);

    LinearSolver::Data data(*device, size, VMA_MEMORY_USAGE_CPU_ONLY);
    Buffer<glm::ivec2> valid(*device, size.x*size.y, VMA_MEMORY_USAGE_CPU_ONLY);
    Pressure pressure(*device, 0.01f, size, data, velocity, solidPhi, liquidPhi, valid);

    Vortex2D::Fluid::Rectangle rectangle(*device, {6.0f, 4.0f});
    Vortex2D::Fluid::RigidBody rigidBody(*device,
                                         Dimensions(size, 1.0f),
                                         rectangle, {3.0f, 2.0f},
                                         solidPhi,
                                         Vortex2D::Fluid::RigidBody::Type::eStatic);

    rigidBody.Anchor = {3.0f, 2.0f};
    rigidBody.Position = {10.0f, 10.0f};
    rigidBody.UpdatePosition();

    rigidBody.RenderPhi();

    rigidBody.SetVelocities(glm::vec2(0.0f, 0.0f), w);
    rigidBody.BindDiv(data.B, data.Diagonal, liquidPhi);
    pressure.BuildLinearEquation();
    rigidBody.Div();
    device->Handle().waitIdle();

    CheckDiv(size, data.B, sim, 1e-3f);
}

TEST(RigidbodyTests, VelocityRotationDiv)
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

    Velocity velocity(*device, size);
    RenderTexture solidPhi(*device, size.x, size.y, vk::Format::eR32Sfloat);
    Texture liquidPhi(*device, size.x, size.y, vk::Format::eR32Sfloat);

    BuildInputs(*device, size, sim, velocity, solidPhi, liquidPhi);
    SetSolidPhi(*device, size, solidPhi, sim, (float)size.x);

    LinearSolver::Data data(*device, size, VMA_MEMORY_USAGE_CPU_ONLY);
    Buffer<glm::ivec2> valid(*device, size.x*size.y, VMA_MEMORY_USAGE_CPU_ONLY);
    Pressure pressure(*device, 0.01f, size, data, velocity, solidPhi, liquidPhi, valid);

    Vortex2D::Fluid::Rectangle rectangle(*device, {6.0f, 4.0f});
    Vortex2D::Fluid::RigidBody rigidBody(*device,
                                         Dimensions(size, 1.0f),
                                         rectangle, {3.0f, 2.0f},
                                         solidPhi,
                                         Vortex2D::Fluid::RigidBody::Type::eStatic);

    rigidBody.Anchor = {3.0f, 2.0f};
    rigidBody.Position = {10.0f, 10.0f};
    rigidBody.UpdatePosition();

    rigidBody.RenderPhi();

    rigidBody.SetVelocities(glm::vec2(v[0], v[1]) * glm::vec2((float)size.x), w);
    rigidBody.BindDiv(data.B, data.Diagonal, liquidPhi);
    pressure.BuildLinearEquation();
    rigidBody.Div();
    device->Handle().waitIdle();

    CheckDiv(size, data.B, sim, 1e-3f);
}

TEST(RigidbodyTests, ReduceJSum)
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

TEST(RigidbodyTests, Pressure)
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
    float w, angular_momentum; Vec2f v;
    sim.rbd->getAngularMomentum(angular_momentum);
    sim.rbd->getAngularVelocity(w);
    sim.rbd->getLinearVelocity(v);

    sim.update_rigid_body_grids();
    sim.add_force(0.01f);

    Velocity velocity(*device, size);
    RenderTexture solidPhi(*device, size.x, size.y, vk::Format::eR32Sfloat);
    Texture liquidPhi(*device, size.x, size.y, vk::Format::eR32Sfloat);

    BuildInputs(*device, size, sim, velocity, solidPhi, liquidPhi);
    SetSolidPhi(*device, size, solidPhi, sim, (float)size.x);

    Buffer<float> pressure(*device, size.x*size.y, VMA_MEMORY_USAGE_CPU_ONLY);

    std::vector<float> computedPressureData(size.x*size.y, 0.0f);
    for (std::size_t i = 0; i < computedPressureData.size(); i++)
    {
        computedPressureData[i] = (float)sim.pressure[i];
    }
    CopyFrom(pressure, computedPressureData);

    Vortex2D::Fluid::Rectangle rectangle(*device, {6.0f, 4.0f});
    Vortex2D::Fluid::RigidBody rigidBody(*device,
                                         Dimensions(size, 1.0f),
                                         rectangle, {3.0, 2.0f},
                                         solidPhi,
                                         Vortex2D::Fluid::RigidBody::Type::eStatic);

    rigidBody.Position = {10.0f, 10.0f};
    rigidBody.Anchor = {3.0f, 2.0f};
    rigidBody.UpdatePosition();

    rigidBody.RenderPhi();

    Buffer<Vortex2D::Fluid::RigidBody::Velocity> force(*device, size.x*size.y, VMA_MEMORY_USAGE_CPU_ONLY);

    rigidBody.BindPressure(liquidPhi, pressure, force);
    rigidBody.Pressure();
    device->Handle().waitIdle();

    float new_angular_momentum; Vec2f new_vel;
    sim.rbd->getLinearVelocity(new_vel);
    sim.rbd->getAngularMomentum(new_angular_momentum);

    auto rigidbodyForce = rigidBody.GetForces();

    Vortex2D::Fluid::RigidBody::Velocity newForce;
    newForce.velocity.x = v[0] + 0.01f * rigidbodyForce.velocity.x / sim.rigid_u_mass;
    newForce.velocity.y = v[1] + 0.01f * rigidbodyForce.velocity.y / sim.rigid_v_mass;
    newForce.angular_velocity = angular_momentum + 0.01f * rigidbodyForce.angular_velocity / size.x;

    EXPECT_NEAR(new_angular_momentum, newForce.angular_velocity, 1e-5f);
    EXPECT_NEAR(new_vel[0], newForce.velocity.x, 1e-5f);
    EXPECT_NEAR(new_vel[1], newForce.velocity.y, 1e-5f);
}

TEST(RigidbodyTests, VelocityConstrain)
{
    glm::ivec2 size(20);

    glm::vec2 solid_velocity(0.1f, -0.6f);

    FluidSim sim;
    sim.initialize(1.0f, size.x, size.y);
    sim.set_boundary(boundary_phi);

    AddParticles(size, sim, boundary_phi);

    // setup rigid body
    sim.rigidgeom = new Box2DGeometry(0.3f, 0.2f);
    sim.rbd = new ::RigidBody(0.4f, *sim.rigidgeom);
    sim.rbd->setCOM(Vec2f(0.5f, 0.5f));
    sim.rbd->setAngle(0.0);

    sim.update_rigid_body_grids();
    sim.add_force(0.01f);
    sim.apply_projection(0.01f);

    // ensure velocities
    sim.rbd->setAngularMomentum(0.0f);
    sim.rbd->setLinearVelocity(Vec2f(solid_velocity.x, solid_velocity.y));

    RenderTexture solidPhi(*device, size.x, size.y, vk::Format::eR32Sfloat);
    // FIXME should set the scale to size.x
    SetSolidPhi(*device, size, solidPhi, sim);

    extrapolate(sim.u, sim.u_valid);
    extrapolate(sim.v, sim.v_valid);

    sim.recompute_solid_velocity();

    Velocity velocity(*device, size);
    SetVelocity(*device, size, velocity, sim);

    sim.constrain_velocity();

    Vortex2D::Fluid::Rectangle rectangle(*device, {6.0f, 4.0f});
    Vortex2D::Fluid::RigidBody rigidBody(*device,
                                         Dimensions(size, 1.0f),
                                         rectangle, {0.0f, 0.0f},
                                         solidPhi,
                                         Vortex2D::Fluid::RigidBody::Type::eStatic);

    rigidBody.Anchor = {3.0f, 2.0f};
    rigidBody.Position = {10.0f, 10.0f};
    rigidBody.UpdatePosition();

    rigidBody.RenderPhi();

    Buffer<glm::ivec2> valid(*device, size.x*size.y, VMA_MEMORY_USAGE_CPU_ONLY);

    Extrapolation extrapolation(*device, size, valid, velocity);
    extrapolation.ConstrainBind(solidPhi);
    extrapolation.ConstrainVelocity();

    rigidBody.SetVelocities(solid_velocity * glm::vec2((float)size.x), 0.0f);
    rigidBody.BindVelocityConstrain(velocity);
    rigidBody.VelocityConstrain();

    device->Handle().waitIdle();

    CheckVelocity(*device, size, velocity, sim, 1e-5f);
}

TEST(RigidbodyTests, RotationConstrain)
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

    sim.update_rigid_body_grids();
    sim.add_force(0.01f);
    sim.apply_projection(0.01f);

    // set velocities
    sim.rbd->setAngularMomentum(-0.34f);
    sim.rbd->setLinearVelocity(Vec2f(0.0f, 0.0f));

    float w;
    sim.rbd->getAngularVelocity(w);

    RenderTexture solidPhi(*device, size.x, size.y, vk::Format::eR32Sfloat);
    // FIXME should set the scale to size.x
    SetSolidPhi(*device, size, solidPhi, sim);

    extrapolate(sim.u, sim.u_valid);
    extrapolate(sim.v, sim.v_valid);

    sim.recompute_solid_velocity();

    Velocity velocity(*device, size);
    SetVelocity(*device, size, velocity, sim);

    sim.constrain_velocity();

    Vortex2D::Fluid::Rectangle rectangle(*device, {6.0f, 4.0f});
    Vortex2D::Fluid::RigidBody rigidBody(*device,
                                         Dimensions(size, 1.0f),
                                         rectangle, {3.0f, 2.0f},
                                         solidPhi,
                                         Vortex2D::Fluid::RigidBody::Type::eStatic);

    rigidBody.Anchor = {3.0f, 2.0f};
    rigidBody.Position = {10.0f, 10.0f};
    rigidBody.UpdatePosition();

    rigidBody.RenderPhi();

    Buffer<glm::ivec2> valid(*device, size.x*size.y, VMA_MEMORY_USAGE_CPU_ONLY);

    Extrapolation extrapolation(*device, size, valid, velocity);
    extrapolation.ConstrainBind(solidPhi);
    extrapolation.ConstrainVelocity();

    rigidBody.SetVelocities(glm::vec2(0.0f, 0.0f), w);
    rigidBody.BindVelocityConstrain(velocity);
    rigidBody.VelocityConstrain();

    device->Handle().waitIdle();

    CheckVelocity(*device, size, velocity, sim, 1e-3f);
}
