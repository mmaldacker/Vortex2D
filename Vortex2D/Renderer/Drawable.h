//
//  Drawable.h
//  Vortex
//

#ifndef Vortex_Drawable_h
#define Vortex_Drawable_h

#include <Vortex2D/Renderer/Common.h>

namespace Vortex2D { namespace Renderer {

class RenderState;

struct Drawable
{
    virtual ~Drawable() {}
    virtual void Initialize(const RenderState& renderState) = 0;
    virtual void Update(const glm::mat4& model, const glm::mat4& view) = 0;
    virtual void Draw(vk::CommandBuffer commandBuffer, const RenderState& renderState) = 0;
};

}}

#endif
