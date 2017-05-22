//
//  Drawable.h
//  Vortex
//

#ifndef Vortex_Drawable_h
#define Vortex_Drawable_h

#include <Vortex2D/Renderer/Common.h>

namespace Vortex2D { namespace Renderer {

class RenderTarget;

struct Drawable
{
    virtual ~Drawable() {}
    virtual void Create(RenderTarget& renderTarget) = 0;
    virtual void Draw(vk::CommandBuffer commandBuffer, vk::RenderPass renderPass) = 0;
};

}}

#endif
