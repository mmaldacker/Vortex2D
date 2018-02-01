//
//  IncompletePoisson.cpp
//  Vortex2D
//

#include "IncompletePoisson.h"

namespace Vortex2D { namespace Fluid {

IncompletePoisson::IncompletePoisson(const Renderer::Device& device, const glm::ivec2& size)
    : mIncompletePoisson(device, size, "IncompletePoisson.comp.spv")
{

}

void IncompletePoisson::Init(Renderer::GenericBuffer& d,
                             Renderer::GenericBuffer& l,
                             Renderer::GenericBuffer& b,
                             Renderer::GenericBuffer& pressure)
{
    mIncompletePoissonBound = mIncompletePoisson.Bind({d, l, b, pressure});
}

void IncompletePoisson::Record(vk::CommandBuffer commandBuffer)
{
    mIncompletePoissonBound.Record(commandBuffer);
}

}}
