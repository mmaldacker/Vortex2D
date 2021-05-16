//
//  IncompletePoisson.h
//  Vortex
//

#pragma once

#include <Vortex2D/Engine/LinearSolver/Preconditioner.h>
#include <Vortex2D/Renderer/CommandBuffer.h>
#include <Vortex2D/Renderer/Work.h>

namespace Vortex
{
namespace Fluid
{
/**
 * @brief Incomplete poisson preconditioner. Slightly better than a simple
 * diagonal preconditioner.
 */
class IncompletePoisson : public Preconditioner
{
public:
  VORTEX_API IncompletePoisson(const Renderer::Device& device, const glm::ivec2& size);
  VORTEX_API ~IncompletePoisson() override;

  VORTEX_API void Bind(Renderer::GenericBuffer& d,
                       Renderer::GenericBuffer& l,
                       Renderer::GenericBuffer& b,
                       Renderer::GenericBuffer& pressure) override;

  void Record(vk::CommandBuffer) override;

private:
  Renderer::Work mIncompletePoisson;
  Renderer::Work::Bound mIncompletePoissonBound;
};

}  // namespace Fluid
}  // namespace Vortex
