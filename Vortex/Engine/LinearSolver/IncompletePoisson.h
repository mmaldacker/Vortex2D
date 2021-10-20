//
//  IncompletePoisson.h
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
 * @brief Incomplete poisson preconditioner. Slightly better than a simple
 * diagonal preconditioner.
 */
class IncompletePoisson : public Preconditioner
{
public:
  VORTEX_API IncompletePoisson(Renderer::Device& device, const glm::ivec2& size);
  VORTEX_API ~IncompletePoisson() override;

  VORTEX_API void Bind(Renderer::GenericBuffer& d,
                       Renderer::GenericBuffer& l,
                       Renderer::GenericBuffer& b,
                       Renderer::GenericBuffer& pressure) override;

  void Record(vk::CommandBuffer) override;

private:
  Renderer::GenericBuffer* mPressure;

  Renderer::Work mIncompletePoisson;
  Renderer::Work::Bound mIncompletePoissonBound;
};

}  // namespace Fluid
}  // namespace Vortex
