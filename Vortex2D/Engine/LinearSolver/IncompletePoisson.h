//
//  IncompletePoisson.h
//  Vortex2D
//

#ifndef Vortex2D_IncompletePoisson_h
#define Vortex2D_IncompletePoisson_h

#include <Vortex2D/Engine/LinearSolver/Preconditioner.h>
#include <Vortex2D/Renderer/Work.h>
#include <Vortex2D/Renderer/CommandBuffer.h>

namespace Vortex2D { namespace Fluid {

class IncompletePoisson : public Preconditioner
{
public:
    IncompletePoisson(const Renderer::Device& device, const glm::ivec2& size);

    void Init(Renderer::Buffer& d,
              Renderer::Buffer& l,
              Renderer::Buffer& b,
              Renderer::Buffer& pressure) override;

    void Record(vk::CommandBuffer ) override;

private:
    Renderer::Work mIncompletePoisson;
    Renderer::Work::Bound mIncompletePoissonBound;
};

}}

#endif
