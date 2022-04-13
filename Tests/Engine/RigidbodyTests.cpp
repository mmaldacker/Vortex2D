//
//  RigidbodyTests.cpp
//  Vortex
//

#include "VariationalHelpers.h"
#include "Verify.h"
#include "box2dgeometry.h"
#include "rigidbody.h"

#include <Vortex/Engine/Boundaries.h>
#include <Vortex/Engine/Extrapolation.h>
#include <Vortex/Engine/LinearSolver/ConjugateGradient.h>
#include <Vortex/Engine/LinearSolver/Diagonal.h>
#include <Vortex/Engine/Pressure.h>
#include <Vortex/Engine/Rigidbody.h>
#include <Vortex/Renderer/RenderTexture.h>

using namespace Vortex::Renderer;
using namespace Vortex::Fluid;

std::ostream& operator<<(std::ostream& o, Vortex::Fluid::RigidBody::Velocity velocity)
{
  return o << velocity.velocity.x << "," << velocity.velocity.y << "," << velocity.angular_velocity;
}

extern Device* device;

void PrintRigidBody(const glm::ivec2& size, FluidSim& sim)
{
  for (int j = 0; j < size.y; j++)
  {
    for (int i = 0; i < size.x; i++)
    {
      std::cout << "(" << sim.nodal_rigid_phi(i, j) * size.x << ")";
    }
    std::cout << std::endl;
  }
  std::cout << std::endl;
}

void PrintForce(const glm::ivec2& size, Buffer<Vortex::Fluid::RigidBody::Velocity>& force)
{
  std::vector<Vortex::Fluid::RigidBody::Velocity> outForce(size.x * size.y);
  CopyTo(force, outForce);

  for (int j = 0; j < size.y; j++)
  {
    for (int i = 0; i < size.x; i++)
    {
      int index = i + j * size.x;
      std::cout << "((" << outForce[index].velocity.x << "," << outForce[index].velocity.y << "),"
                << outForce[index].angular_velocity << ")";
    }

    std::cout << std::endl;
  }

  std::cout << std::endl;
}

void CheckPhi(const glm::ivec2& size, FluidSim& sim, Texture& phi, float error = 1e-5f)
{
  std::vector<float> data(size.x * size.y);
  phi.CopyTo(data);

  for (int i = 0; i < size.x; i++)
  {
    for (int j = 0; j < size.y; j++)
    {
      int index = i + j * size.x;
      EXPECT_NEAR(sim.nodal_rigid_phi(i, j) * size.x, data[index], error);
    }
  }
}

void ProjectParticles(FluidSim& sim)
{
  for (unsigned int p = 0; p < sim.particles.size(); ++p)
  {
    if (sim.rbd)
    {
      sim.rbd->testCollisionAndProject(sim.particles[p], sim.particles[p]);
    }
  }
}

TEST(RigidbodyTests, Phi)
{
  glm::ivec2 size(50);
  glm::vec2 rectangleSize(0.3f, 0.2f);

  FluidSim sim;
  sim.initialize(1.0f, size.x, size.y);

  // setup rigid body
  sim.rigidgeom = new Box2DGeometry(rectangleSize.x, rectangleSize.y);
  sim.rbd = new ::RigidBody(0.4f, *sim.rigidgeom);
  sim.rbd->setCOM(Vec2f(0.5f, 0.5f));
  sim.rbd->setAngle(0.0);

  sim.update_rigid_body_grids();

  RenderTexture solidPhi(*device, size.x, size.y, Format::R32Sfloat);
  device->Execute(
      [&](CommandEncoder& command) {
        solidPhi.Clear(command, std::array<float, 4>{{1000.0f, 0.0f, 0.0f, 0.0f}});
      });

  auto rectangle = std::make_shared<Vortex::Fluid::Rectangle>(
      *device, rectangleSize * glm::vec2(size), false, size.x);
  auto rigidBody = std::make_shared<Vortex::Fluid::RigidBody>(
      *device, size, rectangle, Vortex::Fluid::RigidBody::Type::eStatic);
  rigidBody->BindPhi(solidPhi);

  rigidBody->Anchor = glm::vec2(0.5) * rectangleSize * glm::vec2(size);
  rigidBody->Position = glm::vec2(0.5) * glm::vec2(size);
  rigidBody->UpdatePosition();

  rigidBody->RenderPhi();

  device->WaitIdle();

  Texture outTexture(*device, size.x, size.y, Format::R32Sfloat, MemoryUsage::Cpu);
  device->Execute([&](CommandEncoder& command) { outTexture.CopyFrom(command, solidPhi); });

  CheckPhi(size, sim, outTexture);
}

TEST(RigidbodyTests, Div)
{
  glm::ivec2 size(50);
  glm::vec2 rectangleSize(0.3f, 0.2f);

  FluidSim sim;
  sim.initialize(1.0f, size.x, size.y);
  sim.set_boundary(boundary_phi);

  AddParticles(size, sim, boundary_phi);

  // setup rigid body
  sim.rigidgeom = new Box2DGeometry(rectangleSize.x, rectangleSize.y);
  sim.rbd = new ::RigidBody(0.4f, *sim.rigidgeom);
  sim.rbd->setCOM(Vec2f(0.5f, 0.5f));
  sim.rbd->setAngle(0.0);
  sim.rbd->setAngularMomentum(0.0f);
  sim.rbd->setLinearVelocity(Vec2f(0.0f, 0.0f));

  sim.update_rigid_body_grids();
  sim.add_force(0.01f);

  Velocity velocity(*device, size);
  RenderTexture solidPhi(*device, size.x, size.y, Format::R32Sfloat);
  Texture liquidPhi(*device, size.x, size.y, Format::R32Sfloat);

  BuildInputs(*device, size, sim, velocity, solidPhi, liquidPhi);
  SetSolidPhi(*device, size, solidPhi, sim, (float)size.x);

  LinearSolver::Data data(*device, size, MemoryUsage::Cpu);
  Buffer<glm::ivec2> valid(*device, size.x * size.y, MemoryUsage::Cpu);
  Pressure pressure(*device, 0.01f, size, data, velocity, solidPhi, liquidPhi, valid);

  auto rectangle = std::make_shared<Vortex::Fluid::Rectangle>(
      *device, rectangleSize * glm::vec2(size), false, size.x);
  auto rigidBody = std::make_shared<Vortex::Fluid::RigidBody>(
      *device, size, rectangle, Vortex::Fluid::RigidBody::Type::eStatic);
  rigidBody->BindPhi(solidPhi);

  rigidBody->Anchor = glm::vec2(0.5) * rectangleSize * glm::vec2(size);
  rigidBody->Position = glm::vec2(0.5) * glm::vec2(size);
  rigidBody->UpdatePosition();

  rigidBody->RenderPhi();

  rigidBody->SetVelocities(glm::vec2(0.0f, 0.0f), 0.0f);
  rigidBody->BindDiv(data.B, data.Diagonal);
  pressure.BuildLinearEquation();
  rigidBody->Div();
  device->WaitIdle();

  CheckDiv(size, data.B, sim);
}

TEST(RigidbodyTests, VelocityDiv)
{
  glm::ivec2 size(50);
  glm::vec2 rectangleSize(0.3f, 0.2f);

  FluidSim sim;
  sim.initialize(1.0f, size.x, size.y);
  sim.set_boundary(boundary_phi);

  AddParticles(size, sim, boundary_phi);

  // setup rigid body
  sim.rigidgeom = new Box2DGeometry(rectangleSize.x, rectangleSize.y);
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
  RenderTexture solidPhi(*device, size.x, size.y, Format::R32Sfloat);
  Texture liquidPhi(*device, size.x, size.y, Format::R32Sfloat);

  BuildInputs(*device, size, sim, velocity, solidPhi, liquidPhi);
  SetSolidPhi(*device, size, solidPhi, sim, (float)size.x);

  LinearSolver::Data data(*device, size, MemoryUsage::Cpu);
  Buffer<glm::ivec2> valid(*device, size.x * size.y, MemoryUsage::Cpu);
  Pressure pressure(*device, 0.01f, size, data, velocity, solidPhi, liquidPhi, valid);

  auto rectangle = std::make_shared<Vortex::Fluid::Rectangle>(
      *device, rectangleSize * glm::vec2(size), false, size.x);
  auto rigidBody = std::make_shared<Vortex::Fluid::RigidBody>(
      *device, size, rectangle, Vortex::Fluid::RigidBody::Type::eStatic);
  rigidBody->BindPhi(solidPhi);

  rigidBody->Anchor = glm::vec2(0.5) * rectangleSize * glm::vec2(size);
  rigidBody->Position = glm::vec2(0.5) * glm::vec2(size);
  rigidBody->UpdatePosition();

  rigidBody->RenderPhi();

  // verify div
  rigidBody->SetVelocities(glm::vec2(v[0], v[1]) * glm::vec2(size.x), 0.0f);
  rigidBody->BindDiv(data.B, data.Diagonal);
  pressure.BuildLinearEquation();
  rigidBody->Div();

  device->WaitIdle();

  CheckDiv(size, data.B, sim);
}

TEST(RigidbodyTests, RotationDiv)
{
  glm::ivec2 size(50);
  glm::vec2 rectangleSize(0.3f, 0.2f);

  FluidSim sim;
  sim.initialize(1.0f, size.x, size.y);
  sim.set_boundary(boundary_phi);

  AddParticles(size, sim, boundary_phi);

  // setup rigid body
  sim.rigidgeom = new Box2DGeometry(rectangleSize.x, rectangleSize.y);
  sim.rbd = new ::RigidBody(0.4f, *sim.rigidgeom);
  sim.rbd->setCOM(Vec2f(0.5f, 0.5f));
  sim.rbd->setAngle(0.0);
  sim.rbd->setAngularMomentum(0.1f * sim.rbd->getInertiaModulus());
  sim.rbd->setLinearVelocity(Vec2f(0.0f, 0.0f));

  // get velocities
  float w;
  sim.rbd->getAngularVelocity(w);

  sim.update_rigid_body_grids();
  sim.add_force(0.01f);

  Velocity velocity(*device, size);
  RenderTexture solidPhi(*device, size.x, size.y, Format::R32Sfloat);
  Texture liquidPhi(*device, size.x, size.y, Format::R32Sfloat);

  BuildInputs(*device, size, sim, velocity, solidPhi, liquidPhi);
  SetSolidPhi(*device, size, solidPhi, sim, (float)size.x);

  LinearSolver::Data data(*device, size, MemoryUsage::Cpu);
  Buffer<glm::ivec2> valid(*device, size.x * size.y, MemoryUsage::Cpu);
  Pressure pressure(*device, 0.01f, size, data, velocity, solidPhi, liquidPhi, valid);

  auto rectangle = std::make_shared<Vortex::Fluid::Rectangle>(
      *device, rectangleSize * glm::vec2(size), false, size.x);
  auto rigidBody = std::make_shared<Vortex::Fluid::RigidBody>(
      *device, size, rectangle, Vortex::Fluid::RigidBody::Type::eStatic);
  rigidBody->BindPhi(solidPhi);

  rigidBody->Anchor = glm::vec2(0.5) * rectangleSize * glm::vec2(size);
  rigidBody->Position = glm::vec2(0.5) * glm::vec2(size);
  rigidBody->UpdatePosition();

  rigidBody->RenderPhi();

  rigidBody->SetVelocities(glm::vec2(0.0f, 0.0f), w);
  rigidBody->BindDiv(data.B, data.Diagonal);
  pressure.BuildLinearEquation();
  rigidBody->Div();

  device->WaitIdle();
  CheckDiv(size, data.B, sim, 1e-5f);
}

TEST(RigidbodyTests, VelocityRotationDiv)
{
  glm::ivec2 size(50);
  glm::vec2 rectangleSize(0.3f, 0.2f);

  FluidSim sim;
  sim.initialize(1.0f, size.x, size.y);
  sim.set_boundary(boundary_phi);

  AddParticles(size, sim, boundary_phi);

  // setup rigid body
  sim.rigidgeom = new Box2DGeometry(rectangleSize.x, rectangleSize.y);
  sim.rbd = new ::RigidBody(0.4f, *sim.rigidgeom);
  sim.rbd->setCOM(Vec2f(0.5f, 0.5f));
  sim.rbd->setAngle(0.0);
  sim.rbd->setAngularMomentum(0.1f * sim.rbd->getInertiaModulus());
  sim.rbd->setLinearVelocity(Vec2f(0.1f, 0.0f));

  // get velocities
  float w;
  Vec2f v;
  sim.rbd->getAngularVelocity(w);
  sim.rbd->getLinearVelocity(v);

  sim.update_rigid_body_grids();
  sim.add_force(0.01f);

  Velocity velocity(*device, size);
  RenderTexture solidPhi(*device, size.x, size.y, Format::R32Sfloat);
  Texture liquidPhi(*device, size.x, size.y, Format::R32Sfloat);

  BuildInputs(*device, size, sim, velocity, solidPhi, liquidPhi);
  SetSolidPhi(*device, size, solidPhi, sim, (float)size.x);

  LinearSolver::Data data(*device, size, MemoryUsage::Cpu);
  Buffer<glm::ivec2> valid(*device, size.x * size.y, MemoryUsage::Cpu);
  Pressure pressure(*device, 0.01f, size, data, velocity, solidPhi, liquidPhi, valid);

  auto rectangle = std::make_shared<Vortex::Fluid::Rectangle>(
      *device, rectangleSize * glm::vec2(size), false, size.x);
  auto rigidBody = std::make_shared<Vortex::Fluid::RigidBody>(
      *device, size, rectangle, Vortex::Fluid::RigidBody::Type::eStatic);
  rigidBody->BindPhi(solidPhi);

  rigidBody->Anchor = glm::vec2(0.5) * rectangleSize * glm::vec2(size);
  rigidBody->Position = glm::vec2(0.5) * glm::vec2(size);
  rigidBody->UpdatePosition();

  rigidBody->RenderPhi();

  rigidBody->SetVelocities(glm::vec2(v[0], v[1]) * glm::vec2(size.x), w);
  rigidBody->BindDiv(data.B, data.Diagonal);
  pressure.BuildLinearEquation();
  rigidBody->Div();

  device->WaitIdle();
  CheckDiv(size, data.B, sim, 1e-5f);
}

TEST(RigidbodyTests, ReduceJSum)
{
  glm::ivec2 size(10, 15);
  int n = size.x * size.y;
  Buffer<Vortex::Fluid::RigidBody::Velocity> input(*device, n, MemoryUsage::Cpu);
  Buffer<Vortex::Fluid::RigidBody::Velocity> output(*device, 1, MemoryUsage::Cpu);

  ReduceJ reduce(*device, size.x * size.y);
  auto reduceBound = reduce.Bind(input, output);

  std::vector<Vortex::Fluid::RigidBody::Velocity> inputData(n);

  {
    float n = 1.0f;
    std::generate(inputData.begin(),
                  inputData.end(),
                  [&n] {
                    return Vortex::Fluid::RigidBody::Velocity{glm::vec2(1.0f, -2.0), n++};
                  });
  }

  CopyFrom(input, inputData);

  device->Execute([&](CommandEncoder& command) { reduceBound.Record(command); });

  std::vector<Vortex::Fluid::RigidBody::Velocity> outputData(1);
  CopyTo(output, outputData);

  ASSERT_EQ(0.5f * n * (n + 1), outputData[0].angular_velocity);
  ASSERT_EQ(1.0f * n, outputData[0].velocity.x);
  ASSERT_EQ(-2.0f * n, outputData[0].velocity.y);
}

TEST(RigidbodyTests, Force)
{
  glm::ivec2 size(50);
  glm::vec2 rectangleSize(0.3f, 0.2f);

  FluidSim sim;
  sim.initialize(1.0f, size.x, size.y);
  sim.set_boundary(boundary_phi);

  AddParticles(size, sim, boundary_phi);

  // setup rigid body
  sim.rigidgeom = new Box2DGeometry(rectangleSize.x, rectangleSize.y);
  sim.rbd = new ::RigidBody(0.4f, *sim.rigidgeom);
  sim.rbd->setCOM(Vec2f(0.5f, 0.5f));
  sim.rbd->setAngle(0.0);
  sim.rbd->setAngularMomentum(0.1f * sim.rbd->getInertiaModulus());
  sim.rbd->setLinearVelocity(Vec2f(0.1f, 0.0f));

  // get velocities
  float w, angular_momentum;
  Vec2f v;
  sim.rbd->getAngularMomentum(angular_momentum);
  sim.rbd->getAngularVelocity(w);
  sim.rbd->getLinearVelocity(v);

  sim.update_rigid_body_grids();
  sim.add_force(0.01f);

  Velocity velocity(*device, size);
  RenderTexture solidPhi(*device, size.x, size.y, Format::R32Sfloat);
  Texture liquidPhi(*device, size.x, size.y, Format::R32Sfloat);

  BuildInputs(*device, size, sim, velocity, solidPhi, liquidPhi);
  SetSolidPhi(*device, size, solidPhi, sim, (float)size.x);

  Buffer<float> pressure(*device, size.x * size.y, MemoryUsage::Cpu);
  Buffer<float> diagonal(*device, size.x * size.y, MemoryUsage::Cpu);

  std::vector<float> computedPressureData(size.x * size.y, 0.0f);
  std::vector<float> computedDiagonalData(size.x * size.y, 0.0f);

  for (std::size_t i = 0; i < size.x * size.y; i++)
  {
    computedPressureData[i] = (float)sim.pressure[i];
    computedDiagonalData[i] = (float)sim.matrix(i, i);
  }

  CopyFrom(pressure, computedPressureData);
  CopyFrom(diagonal, computedDiagonalData);

  auto rectangle = std::make_shared<Vortex::Fluid::Rectangle>(
      *device, rectangleSize * glm::vec2(size), false, size.x);
  auto rigidBody = std::make_shared<Vortex::Fluid::RigidBody>(
      *device, size, rectangle, Vortex::Fluid::RigidBody::Type::eWeak);
  rigidBody->BindPhi(solidPhi);

  rigidBody->Anchor = glm::vec2(0.5) * rectangleSize * glm::vec2(size);
  rigidBody->Position = glm::vec2(0.5) * glm::vec2(size);
  rigidBody->UpdatePosition();

  rigidBody->RenderPhi();

  rigidBody->BindForce(diagonal, pressure);
  rigidBody->Force();
  device->WaitIdle();

  float new_angular_momentum;
  Vec2f new_vel;
  sim.rbd->getLinearVelocity(new_vel);
  sim.rbd->getAngularMomentum(new_angular_momentum);

  auto rigidbodyForce = rigidBody->GetForces();

  Vortex::Fluid::RigidBody::Velocity newForce;
  newForce.velocity.x = v[0] + 0.01f * rigidbodyForce.velocity.x / (sim.rigid_u_mass * size.x);
  newForce.velocity.y = v[1] + 0.01f * rigidbodyForce.velocity.y / (sim.rigid_v_mass * size.x);
  newForce.angular_velocity =
      angular_momentum + 0.01f * rigidbodyForce.angular_velocity / (size.x * size.x);

  EXPECT_NEAR(new_angular_momentum, newForce.angular_velocity, 1e-5f);
  EXPECT_NEAR(new_vel[0], newForce.velocity.x, 1e-5f);
  EXPECT_NEAR(new_vel[1], newForce.velocity.y, 1e-5f);
}

TEST(RigidbodyTests, Pressure)
{
  glm::ivec2 size(50);
  glm::vec2 rectangleSize(0.3f, 0.2f);

  FluidSim sim;
  sim.initialize(1.0f, size.x, size.y);
  sim.set_boundary(boundary_phi);

  AddParticles(size, sim, boundary_phi);

  // setup rigid body
  sim.rigidgeom = new Box2DGeometry(rectangleSize.x, rectangleSize.y);
  sim.rbd = new ::RigidBody(0.4f, *sim.rigidgeom);
  sim.rbd->setCOM(Vec2f(0.5f, 0.5f));
  sim.rbd->setAngle(0.0);
  sim.rbd->setAngularMomentum(0.0f);
  sim.rbd->setLinearVelocity(Vec2f(0.0f, 0.0f));
  ProjectParticles(sim);

  sim.update_rigid_body_grids();

  Velocity velocity(*device, size);
  RenderTexture solidPhi(*device, size.x, size.y, Format::R32Sfloat);
  Texture liquidPhi(*device, size.x, size.y, Format::R32Sfloat);

  SetVelocity(*device, size, velocity, sim);

  sim.compute_phi();
  sim.extrapolate_phi();
  sim.u_weights.set_zero();
  sim.v_weights.set_zero();
  sim.rigid_u_mass = sim.rbd->getMass();
  sim.rigid_v_mass = sim.rbd->getMass();
  sim.solve_pressure(0.01f);

  SetLiquidPhi(*device, size, liquidPhi, sim);
  SetSolidPhi(*device, size, solidPhi, sim, (float)size.x);

  LinearSolver::Data data(*device, size, MemoryUsage::Cpu);
  Buffer<glm::ivec2> valid(*device, size.x * size.y, MemoryUsage::Cpu);
  Pressure pressure(*device, 0.01f, size, data, velocity, solidPhi, liquidPhi, valid);

  auto rectangle =
      std::make_shared<Vortex::Fluid::Rectangle>(*device, rectangleSize * glm::vec2(size));
  auto rigidBody = std::make_shared<Vortex::Fluid::RigidBody>(
      *device, size, rectangle, Vortex::Fluid::RigidBody::Type::eStrong);
  rigidBody->BindPhi(solidPhi);
  rigidBody->SetMassData(sim.rbd->getMass(), sim.rbd->getInertiaModulus());

  rigidBody->Anchor = glm::vec2(0.5) * rectangleSize * glm::vec2(size);
  rigidBody->Position = glm::vec2(0.5) * glm::vec2(size);
  rigidBody->UpdatePosition();

  rigidBody->RenderPhi();

  // setup equations
  Buffer<float> input(*device, size.x * size.y, MemoryUsage::Cpu);
  Buffer<float> output(*device, size.x * size.y, MemoryUsage::Cpu);
  device->Execute([&](CommandEncoder& command) { output.Clear(command); });

  std::vector<float> inputData(size.x * size.y, 0.1f);
  CopyFrom(input, inputData);

  pressure.BuildLinearEquation();
  rigidBody->BindPressure(0.01f, data.Diagonal, input, output);

  // multiply matrix
  rigidBody->Pressure();
  device->WaitIdle();

  std::vector<double> outputData(size.x * size.y);
  std::vector<double> inputData2(size.x * size.y, 0.1);
  multiply(sim.matrix, inputData2, outputData);

  CheckPressure(size, outputData, output, 1e-3f);
}

TEST(RigidbodyTests, PressureVelocity)
{
  glm::ivec2 size(50);
  glm::vec2 rectangleSize(0.3f, 0.2f);
  FluidSim sim;
  sim.initialize(1.0f, size.x, size.y);
  sim.set_boundary(boundary_phi);

  AddParticles(size, sim, boundary_phi);

  // setup rigid body
  sim.rigidgeom = new Box2DGeometry(rectangleSize.x, rectangleSize.y);
  sim.rbd = new ::RigidBody(0.4f, *sim.rigidgeom);
  sim.rbd->setCOM(Vec2f(0.5f, 0.5f));
  sim.rbd->setAngle(0.0);
  sim.rbd->setAngularMomentum(0.0f);
  sim.rbd->setLinearVelocity(Vec2f(0.1f, 0.0f));
  ProjectParticles(sim);

  // get velocities
  Vec2f v;
  sim.rbd->getLinearVelocity(v);

  sim.update_rigid_body_grids();
  sim.add_force(0.01f);

  Velocity velocity(*device, size);
  RenderTexture solidPhi(*device, size.x, size.y, Format::R32Sfloat);
  Texture liquidPhi(*device, size.x, size.y, Format::R32Sfloat);

  sim.rigid_u_mass = sim.rbd->getMass();
  sim.rigid_v_mass = sim.rbd->getMass();

  BuildInputs(*device, size, sim, velocity, solidPhi, liquidPhi);
  SetSolidPhi(*device, size, solidPhi, sim, (float)size.x);

  LinearSolver::Data data(*device, size, MemoryUsage::Cpu);

  BuildLinearEquation(size, data.Diagonal, data.Lower, data.B, sim);

  Diagonal preconditioner(*device, size);

  LinearSolver::Parameters params(LinearSolver::Parameters::SolverType::Iterative, 1000, 1e-5f);
  ConjugateGradient solver(*device, size, preconditioner);

  auto rectangle = std::make_shared<Vortex::Fluid::Rectangle>(
      *device, rectangleSize * glm::vec2(size), false, size.x);
  auto rigidBody = std::make_shared<Vortex::Fluid::RigidBody>(
      *device, size, rectangle, Vortex::Fluid::RigidBody::Type::eStrong);
  rigidBody->BindPhi(solidPhi);
  rigidBody->SetMassData(sim.rbd->getMass(), sim.rbd->getInertiaModulus());

  rigidBody->Anchor = glm::vec2(0.5) * rectangleSize * glm::vec2(size);
  rigidBody->Position = glm::vec2(0.5) * glm::vec2(size);
  rigidBody->UpdatePosition();

  rigidBody->RenderPhi();

  // setup equations
  solver.BindRigidbody(0.01f, data.Diagonal, *rigidBody);
  solver.Bind(data.Diagonal, data.Lower, data.B, data.X);

  // solve
  solver.Solve(params, {rigidBody.get()});

  device->WaitIdle();

  std::cout << "Solved in " << params.OutIterations << " iterations. Error " << params.OutError
            << std::endl;

  CheckPressure(size, sim.pressure, data.X, 1e-2f);  // FIXME error is way too high
}

TEST(RigidbodyTests, PressureRotation)
{
  glm::ivec2 size(50);
  glm::vec2 rectangleSize(0.3f, 0.2f);

  FluidSim sim;
  sim.initialize(1.0f, size.x, size.y);
  sim.set_boundary(boundary_phi);

  AddParticles(size, sim, boundary_phi);

  // setup rigid body
  sim.rigidgeom = new Box2DGeometry(rectangleSize.x, rectangleSize.y);
  sim.rbd = new ::RigidBody(0.4f, *sim.rigidgeom);
  sim.rbd->setCOM(Vec2f(0.5f, 0.5f));
  sim.rbd->setAngle(0.0);
  sim.rbd->setAngularMomentum(0.1f * sim.rbd->getInertiaModulus());
  sim.rbd->setLinearVelocity(Vec2f(0.0f, 0.0f));
  ProjectParticles(sim);

  // get velocities
  float w;
  sim.rbd->getAngularVelocity(w);

  sim.update_rigid_body_grids();
  sim.add_force(0.01f);

  Velocity velocity(*device, size);
  RenderTexture solidPhi(*device, size.x, size.y, Format::R32Sfloat);
  Texture liquidPhi(*device, size.x, size.y, Format::R32Sfloat);

  sim.rigid_u_mass = sim.rbd->getMass();
  sim.rigid_v_mass = sim.rbd->getMass();

  BuildInputs(*device, size, sim, velocity, solidPhi, liquidPhi);
  SetSolidPhi(*device, size, solidPhi, sim, (float)size.x);

  LinearSolver::Data data(*device, size, MemoryUsage::Cpu);

  BuildLinearEquation(size, data.Diagonal, data.Lower, data.B, sim);

  Diagonal preconditioner(*device, size);

  LinearSolver::Parameters params(LinearSolver::Parameters::SolverType::Iterative, 1000, 1e-5f);
  ConjugateGradient solver(*device, size, preconditioner);

  auto rectangle = std::make_shared<Vortex::Fluid::Rectangle>(
      *device, rectangleSize * glm::vec2(size), false, size.x);
  auto rigidBody = std::make_shared<Vortex::Fluid::RigidBody>(
      *device, size, rectangle, Vortex::Fluid::RigidBody::Type::eStrong);
  rigidBody->BindPhi(solidPhi);
  rigidBody->SetMassData(sim.rbd->getMass(), sim.rbd->getInertiaModulus());

  rigidBody->Anchor = glm::vec2(0.5) * rectangleSize * glm::vec2(size);
  rigidBody->Position = glm::vec2(0.5) * glm::vec2(size);
  rigidBody->UpdatePosition();

  rigidBody->RenderPhi();

  // setup equations
  solver.BindRigidbody(0.01f, data.Diagonal, *rigidBody);
  solver.Bind(data.Diagonal, data.Lower, data.B, data.X);

  // solve
  solver.Solve(params, {rigidBody.get()});

  device->WaitIdle();

  std::cout << "Solved in " << params.OutIterations << " iterations. Error " << params.OutError
            << std::endl;

  CheckPressure(size, sim.pressure, data.X, 1e-2f);  // FIXME error is way too high
}

TEST(RigidbodyTests, VelocityConstrain)
{
  glm::ivec2 size(50);
  glm::vec2 rectangleSize(0.3f, 0.2f);

  glm::vec2 solid_velocity(0.001f, -0.001f);

  FluidSim sim;
  sim.initialize(1.0f, size.x, size.y);
  sim.set_boundary(boundary_phi);

  AddParticles(size, sim, boundary_phi);

  // setup rigid body
  sim.rigidgeom = new Box2DGeometry(rectangleSize.x, rectangleSize.y);
  sim.rbd = new ::RigidBody(0.4f, *sim.rigidgeom);
  sim.rbd->setCOM(Vec2f(0.5f, 0.5f));
  sim.rbd->setAngle(0.0);

  sim.update_rigid_body_grids();

  // ensure velocities
  sim.rbd->setAngularMomentum(0.0f);
  sim.rbd->setLinearVelocity(Vec2f(solid_velocity.x, solid_velocity.y));

  RenderTexture solidPhi(*device, size.x, size.y, Format::R32Sfloat);
  SetSolidPhi(*device, size, solidPhi, sim, (float)size.x);

  extrapolate(sim.u, sim.u_valid);
  extrapolate(sim.v, sim.v_valid);

  sim.recompute_solid_velocity();
  sim.compute_pressure_weights();

  Velocity velocity(*device, size);
  SetVelocity(*device, size, velocity, sim);

  sim.constrain_velocity();

  auto rectangle = std::make_shared<Vortex::Fluid::Rectangle>(
      *device, rectangleSize * glm::vec2(size), false, size.x);
  auto rigidBody = std::make_shared<Vortex::Fluid::RigidBody>(
      *device, size, rectangle, Vortex::Fluid::RigidBody::Type::eStatic);
  rigidBody->BindPhi(solidPhi);

  rigidBody->Anchor = glm::vec2(0.5) * rectangleSize * glm::vec2(size);
  rigidBody->Position = glm::vec2(0.5) * glm::vec2(size);
  rigidBody->UpdatePosition();

  rigidBody->RenderPhi();

  rigidBody->SetVelocities(solid_velocity * glm::vec2(size.x), 0.0f);
  rigidBody->BindVelocityConstrain(velocity);
  rigidBody->VelocityConstrain();

  device->WaitIdle();

  CheckVelocity(*device, size, velocity, sim, 1e-3f);
}

TEST(RigidbodyTests, RotationConstrain)
{
  glm::ivec2 size(50);
  glm::vec2 rectangleSize(0.3f, 0.2f);

  FluidSim sim;
  sim.initialize(1.0f, size.x, size.y);
  sim.set_boundary(boundary_phi);

  AddParticles(size, sim, boundary_phi);

  // setup rigid body
  sim.rigidgeom = new Box2DGeometry(rectangleSize.x, rectangleSize.y);
  sim.rbd = new ::RigidBody(0.4f, *sim.rigidgeom);
  sim.rbd->setCOM(Vec2f(0.5f, 0.5f));
  sim.rbd->setAngle(0.0);

  sim.update_rigid_body_grids();

  // set velocities
  sim.rbd->setAngularMomentum(0.01f * sim.rbd->getInertiaModulus());
  sim.rbd->setLinearVelocity(Vec2f(0.0f, 0.0f));

  float w;
  sim.rbd->getAngularVelocity(w);

  RenderTexture solidPhi(*device, size.x, size.y, Format::R32Sfloat);
  SetSolidPhi(*device, size, solidPhi, sim, (float)size.x);

  extrapolate(sim.u, sim.u_valid);
  extrapolate(sim.v, sim.v_valid);

  sim.recompute_solid_velocity();
  sim.compute_pressure_weights();

  Velocity velocity(*device, size);
  SetVelocity(*device, size, velocity, sim);

  sim.constrain_velocity();

  auto rectangle = std::make_shared<Vortex::Fluid::Rectangle>(
      *device, rectangleSize * glm::vec2(size), false, size.x);
  auto rigidBody = std::make_shared<Vortex::Fluid::RigidBody>(
      *device, size, rectangle, Vortex::Fluid::RigidBody::Type::eStatic);
  rigidBody->BindPhi(solidPhi);

  rigidBody->Anchor = glm::vec2(0.5) * rectangleSize * glm::vec2(size);
  rigidBody->Position = glm::vec2(0.5) * glm::vec2(size);
  rigidBody->UpdatePosition();

  rigidBody->RenderPhi();

  rigidBody->SetVelocities(glm::vec2(0.0f, 0.0f), w);
  rigidBody->BindVelocityConstrain(velocity);
  rigidBody->VelocityConstrain();

  device->WaitIdle();

  CheckVelocity(*device, size, velocity, sim, 1e-3f);
}
