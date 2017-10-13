//
//  Preconditioner.h
//  Vortex2D
//

#ifndef Vortex2D_Preconditioner_h
#define Vortex2D_Preconditioner_h

#include <Vortex2D/Renderer/Buffer.h>
#include <Vortex2D/Renderer/Texture.h>
#include <Vortex2D/Renderer/Work.h>

namespace Vortex2D { namespace Fluid {

/**
 * @brief An interface to represent a linear solver preconditioner.
 */
struct Preconditioner
{
    virtual ~Preconditioner() {}

    virtual void Init(Renderer::GenericBuffer& d,
                      Renderer::GenericBuffer& l,
                      Renderer::GenericBuffer& b,
                      Renderer::GenericBuffer& x) = 0;

    virtual void Record(vk::CommandBuffer commandBuffer) = 0;
};

}}

#endif
