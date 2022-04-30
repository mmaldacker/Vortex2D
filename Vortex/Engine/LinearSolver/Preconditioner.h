//
//  Preconditioner.h
//  Vortex
//

#pragma once

namespace Vortex
{
namespace Renderer
{
class CommandEncoder;
class GenericBuffer;
}  // namespace Renderer
namespace Fluid
{
/**
 * @brief An interface to represent a linear solver preconditioner.
 */
struct Preconditioner
{
  virtual ~Preconditioner() = default;

  /**
   * @brief Bind the linear equation buffers
   * @param d the diagonal of the matrix
   * @param l the lower matrix
   * @param b the right hand side
   * @param x the unknown buffer
   */
  virtual void Bind(Renderer::GenericBuffer& d,
                    Renderer::GenericBuffer& l,
                    Renderer::GenericBuffer& b,
                    Renderer::GenericBuffer& x) = 0;

  /**
   * @brief Record the preconditioner
   * @param commandBuffer the command buffer to record into.
   */
  virtual void Record(Renderer::CommandEncoder& command) = 0;
};

}  // namespace Fluid
}  // namespace Vortex
