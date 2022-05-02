//
//  IncompletePoisson.cpp
//  Vortex
//

#include "IncompletePoisson.h"

#include "vortex_generated_spirv.h"

namespace Vortex
{
namespace Fluid
{
IncompletePoisson::IncompletePoisson(Renderer::Device& device, const glm::ivec2& size)
    : mIncompletePoisson(device, Renderer::ComputeSize{size}, SPIRV::IncompletePoisson_comp)
{
}

IncompletePoisson::~IncompletePoisson() {}

void IncompletePoisson::Bind(Renderer::GenericBuffer& d,
                             Renderer::GenericBuffer& l,
                             Renderer::GenericBuffer& b,
                             Renderer::GenericBuffer& pressure)
{
  mPressure = &pressure;
  mIncompletePoissonBound = mIncompletePoisson.Bind({d, l, b, pressure});
}

void IncompletePoisson::Record(Renderer::CommandEncoder& command)
{
  assert(mPressure != nullptr);
  mIncompletePoissonBound.Record(command);
  mPressure->Barrier(command, Renderer::Access::Write, Renderer::Access::Read);
}

}  // namespace Fluid
}  // namespace Vortex
