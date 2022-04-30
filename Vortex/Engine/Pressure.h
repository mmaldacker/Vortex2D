//
//  Pressure.h
//  Vortex
//

#pragma once

#include <Vortex/Engine/LinearSolver/LinearSolver.h>
#include <Vortex/Engine/Rigidbody.h>
#include <Vortex/Engine/Velocity.h>
#include <Vortex/Renderer/Buffer.h>
#include <Vortex/Renderer/CommandBuffer.h>
#include <Vortex/Renderer/Texture.h>
#include <Vortex/Renderer/Work.h>

namespace Vortex
{
namespace Fluid
{
/**
 * @brief build the linear equation and compute the divergence from the
 * resulting solution.
 */
class Pressure
{
public:
  VORTEX_API Pressure(Renderer::Device& device,
                      float dt,
                      const glm::ivec2& size,
                      LinearSolver::Data& data,
                      Velocity& velocity,
                      Renderer::Texture& solidPhi,
                      Renderer::Texture& liquidPhi,
                      Renderer::GenericBuffer& valid);

  /**
   * @brief Bind the various buffes for the linear system Ax = b
   * @param size size of the linear system
   * @param diagonal diagonal of A
   * @param lower lower matrix of A
   * @param liquidPhi liquid level set
   * @param solidPhi solid level set
   * @return
   */
  Renderer::Work::Bound BindMatrixBuild(const glm::ivec2& size,
                                        Renderer::GenericBuffer& diagonal,
                                        Renderer::GenericBuffer& lower,
                                        Renderer::Texture& liquidPhi,
                                        Renderer::Texture& solidPhi);

  /**
   * @brief Build the matrix A and right hand side b.
   */
  VORTEX_API void BuildLinearEquation();

  /**
   * @brief Apply the solution of the equation Ax = b, i.e. the pressure to the
   * velocity to make it non-divergent.
   */
  VORTEX_API void ApplyPressure();

private:
  LinearSolver::Data& mData;
  Renderer::Work mBuildMatrix;
  Renderer::Work::Bound mBuildMatrixBound;
  Renderer::Work mBuildDiv;
  Renderer::Work::Bound mBuildDivBound;
  Renderer::Work mProject;
  Renderer::Work::Bound mProjectBound;
  Renderer::CommandBuffer mBuildEquationCmd;
  Renderer::CommandBuffer mProjectCmd;
};

}  // namespace Fluid
}  // namespace Vortex
