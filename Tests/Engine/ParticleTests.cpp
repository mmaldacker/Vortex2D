//
//  ParticleTests.cpp
//  Vortex
//

#include "VariationalHelpers.h"
#include "Verify.h"

#include <glm/gtx/io.hpp>
#include <numeric>
#include <random>

#include <Vortex/Engine/LevelSet.h>
#include <Vortex/Engine/Particles.h>
#include <Vortex/Engine/PrefixScan.h>
#include <Vortex/Renderer/Shapes.h>

using namespace Vortex::Renderer;
using namespace Vortex::Fluid;

extern Device* device;

std::vector<int> GenerateInput(int size)
{
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<int> dist(0, 4);

  std::vector<int> input(size);
  for (int i = 0; i < size; i++)
  {
    input[i] = dist(gen);
  }

  return input;
}

std::vector<int> CalculatePrefixScan(const std::vector<int>& input)
{
  std::vector<int> output(input.size());

  for (unsigned i = 1; i < input.size(); ++i)
  {
    output[i] = input[i - 1] + output[i - 1];
  }

  return output;
}

void PrintPrefixBuffer(const glm::ivec2& size, Buffer<int>& buffer)
{
  int n = size.x * size.y;
  std::vector<int> pixels(n);
  CopyTo(buffer, pixels);

  for (int i = 0; i < n; i++)
  {
    int value = pixels[i];
    std::cout << "(" << value << ")";
  }
  std::cout << std::endl;
}

void PrintPrefixVector(const std::vector<int>& data)
{
  for (auto value : data)
  {
    std::cout << "(" << value << ")";
  }
  std::cout << std::endl;
}

TEST(ParticleTests, PrefixScan)
{
  glm::ivec2 size(20);

  PrefixScan prefixScan(*device, size);

  Buffer<int> input(*device, size.x * size.y, VMA_MEMORY_USAGE_CPU_ONLY);
  Buffer<int> output(*device, size.x * size.y, VMA_MEMORY_USAGE_CPU_ONLY);
  Buffer<DispatchParams> dispatchParams(*device, 1, VMA_MEMORY_USAGE_CPU_ONLY);

  std::vector<int> inputData = GenerateInput(size.x * size.y);
  CopyFrom(input, inputData);

  auto bound = prefixScan.Bind(input, output, dispatchParams);

  device->Execute([&](vk::CommandBuffer commandBuffer) { bound.Record(commandBuffer); });

  auto outputData = CalculatePrefixScan(inputData);
  CheckBuffer(outputData, output);

  DispatchParams params(0);
  CopyTo(dispatchParams, params);

  int total = outputData.back() + inputData.back();
  EXPECT_EQ(params.count, total);
  EXPECT_EQ(params.workSize.x, std::ceil((float)total / 256));
  EXPECT_EQ(params.workSize.y, 1);
  EXPECT_EQ(params.workSize.z, 1);
}

TEST(ParticleTests, PrefixScanBig)
{
  glm::ivec2 size(100);

  PrefixScan prefixScan(*device, size);

  Buffer<int> input(*device, size.x * size.y, VMA_MEMORY_USAGE_CPU_ONLY);
  Buffer<int> output(*device, size.x * size.y, VMA_MEMORY_USAGE_CPU_ONLY);
  Buffer<DispatchParams> dispatchParams(*device, 1, VMA_MEMORY_USAGE_CPU_ONLY);

  std::vector<int> inputData = GenerateInput(size.x * size.y);
  CopyFrom(input, inputData);

  auto bound = prefixScan.Bind(input, output, dispatchParams);

  device->Execute([&](vk::CommandBuffer commandBuffer) { bound.Record(commandBuffer); });

  auto outputData = CalculatePrefixScan(inputData);
  CheckBuffer(outputData, output);

  DispatchParams params(0);
  CopyTo(dispatchParams, params);

  int total = outputData.back() + inputData.back();
  EXPECT_EQ(params.count, total);
  EXPECT_EQ(params.workSize.x, std::ceil((float)total / 256));
  EXPECT_EQ(params.workSize.y, 1);
  EXPECT_EQ(params.workSize.z, 1);
}

TEST(ParticleTests, ParticleCounting)
{
  glm::ivec2 size(20);

  std::vector<Particle> particlesData(size.x * size.y * 8);
  particlesData[0].Position = glm::vec2(3.4f, 2.3f);
  particlesData[1].Position = glm::vec2(3.5f, 2.4f);
  particlesData[2].Position = glm::vec2(5.4f, 6.7f);
  int numParticles = 3;

  Buffer<Particle> particles(*device, 8 * size.x * size.y, VMA_MEMORY_USAGE_CPU_ONLY);
  CopyFrom(particles, particlesData);

  ParticleCount particleCount(
      *device, size, particles, Velocity::InterpolationMode::Cubic, {numParticles});

  particleCount.Scan();
  device->Handle().waitIdle();

  ASSERT_EQ(numParticles, particleCount.GetTotalCount());
}

TEST(ParticleTests, ParticleCounting_OffBounds)
{
  glm::ivec2 size(20);

  std::vector<Particle> particlesData(size.x * size.y * 8);
  particlesData[0].Position = glm::vec2(-3.4f, 2.3f);
  particlesData[1].Position = glm::vec2(3.5f, -2.4f);
  particlesData[2].Position = glm::vec2(55.4f, 6.7f);
  particlesData[3].Position = glm::vec2(5.4f, 46.7f);
  int numParticles = 4;

  Buffer<Particle> particles(*device, 8 * size.x * size.y, VMA_MEMORY_USAGE_CPU_ONLY);
  CopyFrom(particles, particlesData);

  ParticleCount particleCount(
      *device, size, particles, Velocity::InterpolationMode::Cubic, {numParticles});

  particleCount.Scan();
  device->Handle().waitIdle();

  ASSERT_EQ(0, particleCount.GetTotalCount());
}

TEST(ParticleTests, ParticleDelete)
{
  glm::ivec2 size(20);

  std::vector<Particle> particlesData(size.x * size.y * 8);
  particlesData[0].Position = glm::vec2(3.4f, 2.3f);
  particlesData[1].Position = glm::vec2(13.4f, 16.7f);
  particlesData[2].Position = glm::vec2(3.5f, 2.4f);
  int numParticles = 3;

  Buffer<Particle> particles(*device, 8 * size.x * size.y, VMA_MEMORY_USAGE_CPU_ONLY);
  CopyFrom(particles, particlesData);

  ParticleCount particleCount(
      *device, size, particles, Velocity::InterpolationMode::Cubic, {numParticles});

  // Count particles
  particleCount.Scan();
  device->Handle().waitIdle();

  // Delete some particles
  IntRectangle rect(*device, {10, 10});
  rect.Position = glm::vec2(10.0f, 10.0f);
  rect.Colour = glm::vec4(-8);

  particleCount.Record({rect}).Submit();

  // Now scan and copy them
  particleCount.Scan();
  device->Queue().waitIdle();

  ASSERT_EQ(2, particleCount.GetTotalCount());

  // Read particles
  std::vector<Particle> outParticlesData(size.x * size.y * 8);
  CopyTo(particles, outParticlesData);

  // We don't know the order of particles in the same grid position
  EXPECT_TRUE(outParticlesData[0].Position == particlesData[0].Position ||
              outParticlesData[0].Position == particlesData[2].Position);
  EXPECT_TRUE(outParticlesData[1].Position == particlesData[0].Position ||
              outParticlesData[1].Position == particlesData[2].Position);
}

TEST(ParticleTests, ParticleClamp)
{
  glm::ivec2 size(20);

  std::vector<Particle> particlesData(size.x * size.y * 8);

  int numParticles = 10;
  for (int i = 0; i < numParticles; i++)
  {
    particlesData[i].Position = glm::vec2(3.4f, 2.3f);
  }

  Buffer<Particle> particles(*device, 8 * size.x * size.y, VMA_MEMORY_USAGE_CPU_ONLY);
  CopyFrom(particles, particlesData);

  ParticleCount particleCount(
      *device, size, particles, Velocity::InterpolationMode::Cubic, {numParticles});

  particleCount.Scan();
  device->Handle().waitIdle();

  ASSERT_EQ(8, particleCount.GetTotalCount());
}

TEST(ParticleTests, ParticleSpawn)
{
  glm::ivec2 size(20);

  Buffer<Particle> particles(*device, 8 * size.x * size.y, VMA_MEMORY_USAGE_CPU_ONLY);
  ParticleCount particleCount(*device, size, particles, Velocity::InterpolationMode::Cubic);

  // Add some particles
  IntRectangle rect(*device, {1, 1});
  rect.Position = glm::vec2(10.0f, 10.0f);
  rect.Colour = glm::ivec4(4);

  particleCount.Record({rect}).Submit();

  // Now scan and spawn them
  particleCount.Scan();
  device->Queue().waitIdle();

  int particleNum = particleCount.GetTotalCount();
  ASSERT_EQ(4, particleNum);

  // Read particles
  std::vector<Particle> outParticlesData(size.x * size.y * 8);
  CopyTo(particles, outParticlesData);

  glm::ivec2 particlePos(10, 10);
  EXPECT_EQ(glm::ivec2(outParticlesData[0].Position), particlePos);
  EXPECT_EQ(glm::ivec2(outParticlesData[1].Position), particlePos);
  EXPECT_EQ(glm::ivec2(outParticlesData[2].Position), particlePos);
  EXPECT_EQ(glm::ivec2(outParticlesData[3].Position), particlePos);

  std::vector<glm::vec2> outParticles = {outParticlesData[0].Position,
                                         outParticlesData[1].Position,
                                         outParticlesData[2].Position,
                                         outParticlesData[3].Position};
  std::sort(outParticles.begin(), outParticles.end(), [](const auto& left, const auto& right) {
    return std::tie(left.x, left.y) < std::tie(right.x, right.y);
  });
  auto it = std::adjacent_find(outParticles.begin(), outParticles.end());
  ASSERT_EQ(it, outParticles.end());

  for (int i = 0; i < particleNum; i++)
  {
    ASSERT_EQ(outParticlesData[i].Velocity, glm::vec2(0.0));
  }
}

TEST(ParticleTests, ParticleAddDelete)
{
  glm::ivec2 size(20);

  Buffer<Particle> particles(*device, 8 * size.x * size.y, VMA_MEMORY_USAGE_CPU_ONLY);
  ParticleCount particleCount(*device, size, particles, Velocity::InterpolationMode::Cubic);

  // Add some particles
  IntRectangle rectAdd(*device, {2, 4});
  rectAdd.Position = glm::vec2(10.0f, 10.0f);
  rectAdd.Colour = glm::vec4(1);

  particleCount.Record({rectAdd}).Submit();

  // Now scan and spawn them
  particleCount.Scan();
  device->Queue().waitIdle();

  // Remove some particles
  IntRectangle rectRemove(*device, {1, 4});
  rectRemove.Position = glm::vec2(10.0f, 10.0f);
  rectRemove.Colour = glm::ivec4(-8);

  particleCount.Record({rectRemove}).Submit();

  // Scan again
  particleCount.Scan();
  device->Queue().waitIdle();

  // Read particles
  std::vector<Particle> outParticlesData(size.x * size.y * 8);
  CopyTo(particles, outParticlesData);

  ASSERT_EQ(4, particleCount.GetTotalCount());

  EXPECT_EQ(glm::ivec2(outParticlesData[0].Position), glm::ivec2(11, 10));
  EXPECT_EQ(glm::ivec2(outParticlesData[1].Position), glm::ivec2(11, 11));
  EXPECT_EQ(glm::ivec2(outParticlesData[2].Position), glm::ivec2(11, 12));
  EXPECT_EQ(glm::ivec2(outParticlesData[3].Position), glm::ivec2(11, 13));
}

void PrintLiquidPhi(const glm::ivec2& size, FluidSim& sim)
{
  for (int j = 0; j < sim.liquid_phi.nj; j++)
  {
    for (int i = 0; i < sim.liquid_phi.ni; i++)
    {
      std::cout << "(" << size.x * sim.liquid_phi(j, i) << ")";
    }

    std::cout << std::endl;
  }

  std::cout << std::endl;
}

void CheckPhi(const glm::ivec2& size, FluidSim& sim, Texture& phi)
{
  std::vector<float> pixels(size.x * size.y);
  phi.CopyTo(pixels);

  // TODO should check whole texture
  const int off = 3;
  for (int i = off; i < size.x - off; i++)
  {
    for (int j = off; j < size.y - off; j++)
    {
      int index = i + j * size.x;
      float value = pixels[index];
      EXPECT_NEAR(value, size.x * sim.liquid_phi(i, j), 1e-5f) << "Mismatch at " << i << "," << j;
    }
  }
}

TEST(ParticleTests, Phi)
{
  // NOTE Cannot use higher size because of weird float conversions in FluidSim
  glm::ivec2 size(20);

  FluidSim sim;
  sim.initialize(1.0f, size.x, size.y);
  sim.set_boundary(boundary_phi);

  AddParticles(size, sim, boundary_phi);
  sim.compute_phi();

  Buffer<Particle> particles(*device, 8 * size.x * size.y, VMA_MEMORY_USAGE_CPU_ONLY);

  std::vector<Particle> particlesData;
  for (auto& p : sim.particles)
  {
    Particle particle;
    particle.Position = glm::vec2(p[0] * size.x, p[1] * size.x);
    particlesData.push_back(particle);
  }
  particlesData.resize(8 * size.x * size.y);
  CopyFrom(particles, particlesData);

  ParticleCount particleCount(
      *device, size, particles, Velocity::InterpolationMode::Cubic, {(int)sim.particles.size()});

  particleCount.Scan();
  device->Handle().waitIdle();

  LevelSet phi(*device, size);

  particleCount.LevelSetBind(phi);
  particleCount.Phi();
  device->Handle().waitIdle();

  Texture outTexture(*device, size.x, size.y, vk::Format::eR32Sfloat, VMA_MEMORY_USAGE_CPU_ONLY);
  device->Execute(
      [&](vk::CommandBuffer commandBuffer) { outTexture.CopyFrom(commandBuffer, phi); });

  CheckPhi(size, sim, outTexture);
}

TEST(ParticleTests, FromGrid_PIC)
{
  // Small size otherwise test is too slow (due to O(n^2) search)
  glm::ivec2 size(20);

  float alpha = 1.0f;

  // setup FluidSim
  FluidSim sim;
  sim.initialize(1.0f, size.x, size.y);
  sim.set_boundary(boundary_phi);

  AddParticles(size, sim, boundary_phi);

  sim.advance(0.01f);
  sim.update_from_grid(1.0f);

  // setup ParticleCount
  Buffer<Particle> particles(*device, 8 * size.x * size.y, VMA_MEMORY_USAGE_CPU_ONLY);

  std::vector<Particle> particlesData;
  for (std::size_t p = 0; p < sim.particles.size(); p++)
  {
    Particle particle;
    particle.Position = glm::vec2(sim.particles[p][0] * size.x, sim.particles[p][1] * size.x);
    particle.Velocity = glm::vec2(0.0f);
    particlesData.push_back(particle);
  }
  particlesData.resize(8 * size.x * size.y);
  CopyFrom(particles, particlesData);

  ParticleCount particleCount(*device,
                              size,
                              particles,
                              Velocity::InterpolationMode::Cubic,
                              {(int)sim.particles.size()},
                              alpha);

  particleCount.Scan();
  device->Handle().waitIdle();

  ASSERT_EQ(sim.particles.size(), particleCount.GetTotalCount());

  // FromGrid test
  Velocity velocity(*device, size);
  Buffer<glm::ivec2> valid(*device, size.x * size.y, VMA_MEMORY_USAGE_CPU_ONLY);

  SetVelocity(*device, size, velocity, sim);

  particleCount.VelocitiesBind(velocity, valid);
  particleCount.TransferFromGrid();
  device->Handle().waitIdle();

  // Verify particle velocities

  // TODO check valid

  std::vector<Particle> outParticlesData(size.x * size.y * 8);
  CopyTo(particles, outParticlesData);

  for (std::size_t i = 0; i < sim.particles.size(); i++)
  {
    std::size_t index = static_cast<std::size_t>(-1);
    for (std::size_t j = 0; j < sim.particles.size(); j++)
    {
      glm::vec2 pos(sim.particles[j][0] * size.x, sim.particles[j][1] * size.x);
      if (pos == outParticlesData[i].Position)
      {
        index = j;
      }
    }

    ASSERT_NE(static_cast<std::size_t>(-1), index);

    glm::vec2 vel(sim.particles_velocity[index][0], sim.particles_velocity[index][1]);
    EXPECT_NEAR(vel.x, outParticlesData[i].Velocity.x, 1e-5f);
    EXPECT_NEAR(vel.y, outParticlesData[i].Velocity.y, 1e-5f);
  }
}

TEST(ParticleTests, FromGrid_FLIP)
{
  // Small size otherwise test is too slow (due to O(n^2) search)
  glm::ivec2 size(20);

  float alpha = 0.0f;

  // setup FluidSim
  FluidSim sim;
  sim.initialize(1.0f, size.x, size.y);
  sim.set_boundary(boundary_phi);

  AddParticles(size, sim, boundary_phi);

  sim.advance(0.01f);
  sim.get_velocity_update();
  sim.update_from_grid(1.0f);

  // setup ParticleCount
  Buffer<Particle> particles(*device, 8 * size.x * size.y, VMA_MEMORY_USAGE_CPU_ONLY);

  std::vector<Particle> particlesData;
  for (std::size_t p = 0; p < sim.particles.size(); p++)
  {
    Particle particle;
    particle.Position = glm::vec2(sim.particles[p][0] * size.x, sim.particles[p][1] * size.x);
    particle.Velocity = glm::vec2(0.0f);
    particlesData.push_back(particle);
  }
  particlesData.resize(8 * size.x * size.y);
  CopyFrom(particles, particlesData);

  ParticleCount particleCount(*device,
                              size,
                              particles,
                              Velocity::InterpolationMode::Cubic,
                              {(int)sim.particles.size()},
                              alpha);

  particleCount.Scan();
  device->Handle().waitIdle();

  ASSERT_EQ(sim.particles.size(), particleCount.GetTotalCount());

  // FromGrid test
  Velocity velocity(*device, size);
  Buffer<glm::ivec2> valid(*device, size.x * size.y, VMA_MEMORY_USAGE_CPU_ONLY);

  SetVelocity(*device, size, velocity, sim);

  particleCount.VelocitiesBind(velocity, valid);

  velocity.VelocityDiff();
  particleCount.TransferFromGrid();
  device->Handle().waitIdle();

  // Verify particle velocities

  // TODO check valid

  std::vector<Particle> outParticlesData(size.x * size.y * 8);
  CopyTo(particles, outParticlesData);

  for (std::size_t i = 0; i < sim.particles.size(); i++)
  {
    std::size_t index = static_cast<std::size_t>(-1);
    for (std::size_t j = 0; j < sim.particles.size(); j++)
    {
      glm::vec2 pos(sim.particles[j][0] * size.x, sim.particles[j][1] * size.x);
      if (pos == outParticlesData[i].Position)
      {
        index = j;
      }
    }

    ASSERT_NE(static_cast<std::size_t>(-1), index);

    glm::vec2 vel(sim.particles_velocity[index][0], sim.particles_velocity[index][1]);
    EXPECT_NEAR(vel.x, outParticlesData[i].Velocity.x, 1e-5f);
    EXPECT_NEAR(vel.y, outParticlesData[i].Velocity.y, 1e-5f);
  }
}

TEST(ParticleTests, ToGrid)
{
  glm::ivec2 size(50);

  float alpha = 1.0f;

  // setup FluidSim
  FluidSim sim;
  sim.initialize(1.0f, size.x, size.y);
  sim.set_boundary(boundary_phi);

  AddParticles(size, sim, boundary_phi);

  sim.advance(0.01f);
  sim.get_velocity_update();
  sim.update_from_grid(alpha);
  sim.v.set_zero();
  sim.u.set_zero();

  sim.transfer_to_grid();

  // setup ParticleCount
  Buffer<Particle> particles(*device, 8 * size.x * size.y, VMA_MEMORY_USAGE_CPU_ONLY);

  std::vector<Particle> particlesData;
  for (std::size_t p = 0; p < sim.particles.size(); p++)
  {
    Particle particle;
    particle.Position = glm::vec2(sim.particles[p][0] * size.x, sim.particles[p][1] * size.x);
    particle.Velocity = glm::vec2(sim.particles_velocity[p][0], sim.particles_velocity[p][1]);
    particlesData.push_back(particle);
  }
  particlesData.resize(8 * size.x * size.y);
  CopyFrom(particles, particlesData);

  ParticleCount particleCount(*device,
                              size,
                              particles,
                              Velocity::InterpolationMode::Cubic,
                              {(int)sim.particles.size()},
                              alpha);

  particleCount.Scan();
  device->Handle().waitIdle();

  ASSERT_EQ(sim.particles.size(), particleCount.GetTotalCount());

  // ToGrid test
  Velocity velocity(*device, size);
  Buffer<glm::ivec2> valid(*device, size.x * size.y, VMA_MEMORY_USAGE_CPU_ONLY);

  particleCount.VelocitiesBind(velocity, valid);
  particleCount.TransferToGrid();
  device->Handle().waitIdle();

  // TODO check valid

  CheckVelocity(*device, size, velocity, sim, 1e-5f);
}
