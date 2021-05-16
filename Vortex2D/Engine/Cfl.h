//
//  Cfl.h
//  Vertex2D
//

#ifndef Vertex2D_Cfl_h
#define Vertex2D_Cfl_h

#include <Vortex2D/Engine/LinearSolver/Reduce.h>
#include <Vortex2D/Engine/Velocity.h>
#include <Vortex2D/Renderer/CommandBuffer.h>
#include <Vortex2D/Renderer/Work.h>

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
  VORTEX2D_API Cfl(const Renderer::Device& device, const glm::ivec2& size, Velocity& velocity);

  /**
   * Compute the CFL number. Non-blocking.
   */
  VORTEX2D_API void Compute();

  /**
   * Returns the CFL number. Blocking.
   * @return cfl number
   */
  VORTEX2D_API float Get();

private:
  const Renderer::Device& mDevice;
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
}  // namespace Vortex2D

#endif
