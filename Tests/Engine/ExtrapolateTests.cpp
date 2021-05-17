//
//  ExtrapolateTests.cpp
//  Vortex
//

#include <Vortex/Engine/Extrapolation.h>
#include "VariationalHelpers.h"
#include "Verify.h"

using namespace Vortex::Fluid;
using namespace Vortex::Renderer;

extern Device* device;

void PrintValid(const glm::ivec2& size, Vortex::Renderer::Buffer<glm::ivec2>& buffer)
{
  std::vector<glm::ivec2> pixels(size.x * size.y);
  CopyTo(buffer, pixels);

  for (int j = 0; j < size.x; j++)
  {
    for (int i = 0; i < size.y; i++)
    {
      glm::ivec2 value = pixels[i + j * size.x];
      std::cout << "(" << value.x << "," << value.y << ")";
    }
    std::cout << std::endl;
  }
  std::cout << std::endl;
}

void SetValid(const glm::ivec2& size, FluidSim& sim, Buffer<glm::ivec2>& buffer)
{
  std::vector<glm::ivec2> validData(size.x * size.y);

  for (int i = 0; i < size.x; i++)
  {
    for (int j = 0; j < size.y; j++)
    {
      std::size_t index = i + j * size.x;
      validData[index].x = sim.u_valid(i, j);
      validData[index].y = sim.v_valid(i, j);
    }
  }

  CopyFrom(buffer, validData);
}

TEST(ExtrapolateTest, Extrapolate)
{
  glm::ivec2 size(50);

  FluidSim sim;
  sim.initialize(1.0f, size.x, size.y);
  sim.set_boundary(complex_boundary_phi);

  AddParticles(size, sim, complex_boundary_phi);

  sim.add_force(0.01f);
  sim.apply_projection(0.01f);

  Buffer<glm::ivec2> valid(*device, size.x * size.y, VMA_MEMORY_USAGE_CPU_ONLY);
  SetValid(size, sim, valid);

  Velocity velocity(*device, size);
  SetVelocity(*device, size, velocity, sim);

  extrapolate(sim.u, sim.u_valid);
  extrapolate(sim.v, sim.v_valid);

  Extrapolation extrapolation(*device, size, valid, velocity, 10);
  extrapolation.Extrapolate();

  device->Queue().waitIdle();

  CheckVelocity(*device, size, velocity, sim);
  CheckValid(size, sim, valid);
}

TEST(ExtrapolateTest, Constrain)
{
  // FIXME increase size
  glm::ivec2 size(20);

  FluidSim sim;
  sim.initialize(1.0f, size.x, size.y);
  sim.set_boundary(complex_boundary_phi);

  AddParticles(size, sim, complex_boundary_phi);

  sim.add_force(0.01f);
  sim.apply_projection(0.01f);

  Buffer<glm::ivec2> valid(*device, size.x * size.y, VMA_MEMORY_USAGE_CPU_ONLY);

  Texture solidPhi(*device, size.x, size.y, vk::Format::eR32Sfloat);
  SetSolidPhi(*device, size, solidPhi, sim, (float)size.x);

  extrapolate(sim.u, sim.u_valid);
  extrapolate(sim.v, sim.v_valid);

  Velocity velocity(*device, size);
  SetVelocity(*device, size, velocity, sim);

  sim.constrain_velocity();

  Extrapolation extrapolation(*device, size, valid, velocity);
  extrapolation.ConstrainBind(solidPhi);
  extrapolation.ConstrainVelocity();

  device->Queue().waitIdle();

  CheckVelocity(*device, size, velocity, sim, 1e-3f);  // FIXME reduce error tolerance
}
