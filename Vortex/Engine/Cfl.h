//
//  Cfl.h
//  Vortex
//

#pragma once

#include <Vortex/Engine/LinearSolver/Reduce.h>
#include <Vortex/Engine/Velocity.h>
#include <Vortex/Renderer/CommandBuffer.h>
#include <Vortex/Renderer/Work.h>

namespace Vortex
{
namespace Fluid
{
/**
 * Calculates the CFL number of the velocity field. It's an indication on how to
 * choose your time step size. Ideally, the time step should be smaller than the
 * CFL number.
 */
class Cfl
{
public:
  VORTEX_API Cfl(Renderer::Device& device, const glm::ivec2& size, Velocity& velocity);

  /**
   * Compute the CFL number. Non-blocking.
   */
  VORTEX_API void Compute();

  /**
   * Returns the CFL number. Blocking.
   * @return cfl number
   */
  VORTEX_API float Get();

private:
  Renderer::Device& mDevice;
  glm::ivec2 mSize;
  Velocity& mVelocity;
  Renderer::Work mVelocityMaxWork;
  Renderer::Work::Bound mVelocityMaxBound;
  Renderer::Buffer<float> mVelocityMax, mCfl;
  Renderer::CommandBuffer mVelocityMaxCmd;
  ReduceMax mReduceVelocityMax;
  ReduceMax::Bound mReduceVelocityMaxBound;
};

}  // namespace Fluid
}  // namespace Vortex
