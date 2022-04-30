//
//  AdvectionTests.cpp
//  Vortex
//

#include <Vortex/Engine/Advection.h>
#include <Vortex/Engine/Density.h>
#include <Vortex/Engine/Particles.h>
#include "VariationalHelpers.h"
#include "Verify.h"

using namespace Vortex::Renderer;
using namespace Vortex::Fluid;

extern Device* device;

TEST(AdvectionTests, AdvectVelocity_Simple)
{
  glm::ivec2 size(50);

  FluidSim sim;
  sim.initialize(1.0f, size.x, size.y);
  sim.set_boundary(boundary_phi);

  AddParticles(size, sim, boundary_phi);

  sim.advance(0.01f);

  Velocity velocity(*device, size);
  SetVelocity(*device, size, velocity, sim);

  sim.advect(0.01f);

  Advection advection(*device, size, 0.01f, velocity, Velocity::InterpolationMode::Cubic);
  advection.AdvectVelocity();

  device->WaitIdle();

  CheckVelocity(*device, size, velocity, sim, 1e-5f);
}

TEST(AdvectionTests, AdvectVelocity_Complex)
{
  glm::ivec2 size(50);

  FluidSim sim;
  sim.initialize(1.0f, size.x, size.y);
  sim.set_boundary(complex_boundary_phi);

  AddParticles(size, sim, complex_boundary_phi);

  sim.add_force(0.01f);
  sim.advance(0.01f);

  Velocity velocity(*device, size);
  SetVelocity(*device, size, velocity, sim);

  sim.advect(0.01f);

  Advection advection(*device, size, 0.01f, velocity, Velocity::InterpolationMode::Cubic);
  advection.AdvectVelocity();

  device->WaitIdle();

  CheckVelocity(*device, size, velocity, sim, 1e-5f);
}

TEST(AdvectionTests, Advect)
{
  glm::ivec2 size(10);

  glm::vec2 vel(3.0f, 1.0f);
  glm::ivec2 pos(3, 4);

  Texture velocityInput(*device, size.x, size.y, Format::R32G32Sfloat, MemoryUsage::Cpu);
  Velocity velocity(*device, size);

  std::vector<glm::vec2> velocityData(size.x * size.y, vel / glm::vec2(size));
  velocityInput.CopyFrom(velocityData);

  device->Execute([&](CommandEncoder& command) { velocity.CopyFrom(command, velocityInput); });

  Texture fieldInput(*device, size.x, size.y, Format::B8G8R8A8Unorm, MemoryUsage::Cpu);
  Density field(*device, size, Format::B8G8R8A8Unorm);

  std::vector<glm::u8vec4> fieldData(size.x * size.y);
  fieldData[pos.x + size.x * pos.y].x = 128;
  fieldInput.CopyFrom(fieldData);

  device->Execute([&](CommandEncoder& command) { field.CopyFrom(command, fieldInput); });

  Advection advection(*device, size, 1.0f, velocity, Velocity::InterpolationMode::Cubic);
  advection.AdvectBind(field);
  advection.Advect();

  device->WaitIdle();

  device->Execute([&](CommandEncoder& command) { fieldInput.CopyFrom(command, field); });

  std::vector<glm::u8vec4> pixels(fieldInput.GetWidth() * fieldInput.GetHeight());
  fieldInput.CopyTo(pixels);

  pos += glm::ivec2(vel);
  ASSERT_EQ(128, pixels[pos.x + size.x * pos.y].x);
}

TEST(AdvectionTests, ParticleAdvect)
{
  glm::ivec2 size(50);

  FluidSim sim;
  sim.initialize(1.0f, size.x, size.y);
  sim.set_boundary(boundary_phi);

  AddParticles(size, sim, boundary_phi);

  sim.advance(0.01f);

  // setup particles
  Buffer<Particle> particles(*device, 8 * size.x * size.y, MemoryUsage::Cpu);
  IndirectBuffer<DispatchParams> dispatchParams(*device, MemoryUsage::Cpu);

  DispatchParams params(static_cast<int32_t>(sim.particles.size()));
  CopyFrom(dispatchParams, params);

  std::vector<Particle> particlesData;
  for (auto& p : sim.particles)
  {
    Particle particle;
    particle.Position = glm::vec2(p[0] * size.x, p[1] * size.x);
    particlesData.push_back(particle);
  }
  particlesData.resize(8 * size.x * size.y);
  CopyFrom(particles, particlesData);

  // setup velocities
  Velocity velocity(*device, size);

  SetVelocity(*device, size, velocity, sim);

  // setup level set
  Texture solidPhi(*device, size.x, size.y, Format::R32Sfloat);
  SetSolidPhi(*device, size, solidPhi, sim, (float)size.x);

  // advection
  Advection advection(*device, size, 0.01f, velocity, Velocity::InterpolationMode::Cubic);
  advection.AdvectParticleBind(particles, solidPhi, dispatchParams);
  advection.AdvectParticles();
  device->WaitIdle();

  // test
  sim.advect_particles(0.01f);

  std::vector<Particle> outParticlesData(size.x * size.y * 8);
  CopyTo(particles, outParticlesData);

  for (std::size_t i = 0; i < sim.particles.size(); i++)
  {
    glm::vec2 pos(sim.particles[i][0] * size.x, sim.particles[i][1] * size.x);

    EXPECT_NEAR(pos.x, outParticlesData[i].Position.x, 1e-5f);
    EXPECT_NEAR(pos.y, outParticlesData[i].Position.y, 1e-5f);
  }
}

TEST(AdvectionTests, ParticleProject)
{
  glm::ivec2 size(50);

  FluidSim sim;
  sim.initialize(1.0f, size.x, size.y);
  sim.set_boundary(boundary_phi);

  Vec2f left(3.0f / size.x, 3.0f / size.y);
  Vec2f right((size.x - 3.0f) / size.x, 3.0f / size.y);
  Vec2f bottomLeft(3.0f / size.x, (size.y - 3.0f) / size.y);
  Vec2f bottomRight((size.x - 3.0f) / size.x, (size.y - 3.0f) / size.y);

  sim.add_particle(left);
  sim.add_particle(right);
  sim.add_particle(bottomLeft);
  sim.add_particle(bottomRight);

  // setup particles
  Buffer<Particle> particles(*device, 8 * size.x * size.y, MemoryUsage::Cpu);
  IndirectBuffer<DispatchParams> dispatchParams(*device, MemoryUsage::Cpu);

  DispatchParams params(static_cast<int32_t>(sim.particles.size()));
  CopyFrom(dispatchParams, params);

  std::vector<Particle> particlesData;
  for (auto& p : sim.particles)
  {
    Particle particle;
    particle.Position = glm::vec2(p[0] * size.x, p[1] * size.x);
    particlesData.push_back(particle);
  }
  particlesData.resize(8 * size.x * size.y);
  CopyFrom(particles, particlesData);

  // setup velocities
  Velocity velocity(*device, size);

  // setup level set
  Texture solidPhi(*device, size.x, size.y, Format::R32Sfloat);
  SetSolidPhi(*device, size, solidPhi, sim, (float)size.x);

  // advection
  Advection advection(*device, size, 0.01f, velocity, Velocity::InterpolationMode::Cubic);
  advection.AdvectParticleBind(particles, solidPhi, dispatchParams);
  advection.AdvectParticles();
  device->WaitIdle();

  // test
  sim.advect_particles(0.01f);

  std::vector<Particle> outParticlesData(size.x * size.y * 8);
  CopyTo(particles, outParticlesData);

  for (std::size_t i = 0; i < sim.particles.size(); i++)
  {
    glm::vec2 pos(sim.particles[i][0] * size.x, sim.particles[i][1] * size.x);

    EXPECT_NEAR(pos.x, outParticlesData[i].Position.x, 1e-5f);
    EXPECT_NEAR(pos.y, outParticlesData[i].Position.y, 1e-5f);
  }
}
