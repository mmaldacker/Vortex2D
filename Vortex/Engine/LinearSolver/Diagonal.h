//
//  Diagonal.h
//  Vortex
//

#pragma once

#include <Vortex/Engine/LinearSolver/Preconditioner.h>
#include <Vortex/Renderer/CommandBuffer.h>
#include <Vortex/Renderer/Work.h>

namespace Vortex
{
namespace Fluid
{
/**
 * @brief Diagonal preconditioner. Simplest of preconditioner, useful to verify
 * if the preconditioned conjugate gradient works.
 */
class Diagonal : public Preconditioner
{
public:
  VORTEX_API Diagonal(Renderer::Device& device, const glm::ivec2& size);
  VORTEX_API ~Diagonal() override;

  VORTEX_API void Bind(Renderer::GenericBuffer& d,
                       Renderer::GenericBuffer& l,
                       Renderer::GenericBuffer& b,
                       Renderer::GenericBuffer& pressure) override;

  void Record(vk::CommandBuffer) override;

private:
  Renderer::GenericBuffer* mPressure;

  Renderer::Work mDiagonal;
  Renderer::Work::Bound mDiagonalBound;
};

}  // namespace Fluid
}  // namespace Vortex
