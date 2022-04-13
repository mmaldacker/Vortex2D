//
//  GaussSeidel.h
//  Vortex
//

#pragma once

#include <Vortex/Engine/LinearSolver/LinearSolver.h>
#include <Vortex/Engine/LinearSolver/Preconditioner.h>
#include <Vortex/Renderer/CommandBuffer.h>
#include <Vortex/Renderer/Pipeline.h>
#include <Vortex/Renderer/Work.h>

namespace Vortex
{
namespace Fluid
{
/**
 * @brief An iterative black and red successive over relaxation linear solver.
 */
class GaussSeidel : public LinearSolver, public Preconditioner
{
public:
  VORTEX_API GaussSeidel(Renderer::Device& device, const glm::ivec2& size);
  VORTEX_API ~GaussSeidel() override;

  VORTEX_API void Bind(Renderer::GenericBuffer& d,
                       Renderer::GenericBuffer& l,
                       Renderer::GenericBuffer& b,
                       Renderer::GenericBuffer& pressure) override;

  VORTEX_API void BindRigidbody(float delta,
                                Renderer::GenericBuffer& d,
                                RigidBody& rigidBody) override;

  /**
   * @brief Iterative solving of the linear equations in data
   */
  VORTEX_API void Solve(Parameters& params,
                        const std::vector<RigidBody*>& rigidbodies = {}) override;

  VORTEX_API float GetError() override;

  void Record(Renderer::CommandEncoder& commandEncoder) override;

  /**
   * @brief Record a determined number of iterations
   * @param commandBuffer
   * @param iterations
   */
  void Record(Renderer::CommandEncoder& command, int iterations);

  /**
   * @brief Set the w factor of the GS iterations : x_new = w * x_new + (1-w) *
   * x_old
   * @param w
   */
  VORTEX_API void SetW(float w);

  /**
   * @brief set number of iterations to be used when GS is a preconditioner
   * @param iterations
   */
  VORTEX_API void SetPreconditionerIterations(int iterations);

private:
  float mW;
  int mPreconditionerIterations;

  LinearSolver::Error mError;

  Renderer::Work mGaussSeidel;
  Renderer::Work::Bound mGaussSeidelBound;

  Renderer::CommandBuffer mInitCmd;
  Renderer::CommandBuffer mGaussSeidelCmd;
  Renderer::GenericBuffer* mPressure;
};

/**
 * @brief A version of the gauss seidel that can only be applied on sizes
 * (16,16) or smaller.
 */
class LocalGaussSeidel : public Preconditioner
{
public:
  VORTEX_API LocalGaussSeidel(Renderer::Device& device, const glm::ivec2& size);
  VORTEX_API ~LocalGaussSeidel() override;

  void VORTEX_API Bind(Renderer::GenericBuffer& d,
                       Renderer::GenericBuffer& l,
                       Renderer::GenericBuffer& b,
                       Renderer::GenericBuffer& pressure) override;

  void Record(Renderer::CommandEncoder& commandEncoder) override;

private:
  Renderer::GenericBuffer* mPressure;

  Renderer::Work mLocalGaussSeidel;
  Renderer::Work::Bound mLocalGaussSeidelBound;
};

}  // namespace Fluid
}  // namespace Vortex
