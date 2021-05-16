//
//  Jacobi.h
//  Vortex
//

#pragma once

#include <Vortex/Engine/LinearSolver/LinearSolver.h>
#include <Vortex/Engine/LinearSolver/Preconditioner.h>
#include <Vortex/Engine/LinearSolver/Reduce.h>
#include <Vortex/Renderer/CommandBuffer.h>
#include <Vortex/Renderer/Pipeline.h>
#include <Vortex/Renderer/Work.h>

namespace Vortex
{
namespace Fluid
{
/**
 * @brief An iterative jacobi linear solver.
 */
class Jacobi : public Preconditioner
{
public:
  Jacobi(const Renderer::Device& device, const glm::ivec2& size);

  void Bind(Renderer::GenericBuffer& d,
            Renderer::GenericBuffer& l,
            Renderer::GenericBuffer& b,
            Renderer::GenericBuffer& pressure) override;

  void Record(vk::CommandBuffer commandBuffer) override;

  void Record(vk::CommandBuffer commandBuffer, int iterations);

  /**
   * @brief Set the w factor of the GS iterations : x_new = w * x_new + (1-w) *
   * x_old
   * @param w
   */
  void SetW(float w);

  /**
   * @brief set number of iterations to be used when GS is a preconditioner
   * @param iterations
   */
  void SetPreconditionerIterations(int iterations);

private:
  float mW;
  int mPreconditionerIterations;

  Renderer::GenericBuffer* mPressure;
  Renderer::Buffer<float> mBackPressure;

  Renderer::Work mJacobi;
  Renderer::Work::Bound mJacobiFrontBound;
  Renderer::Work::Bound mJacobiBackBound;
};

}  // namespace Fluid
}  // namespace Vortex
