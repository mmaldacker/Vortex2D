//
//  RenderTarget.h
//  Vortex2D
//

#ifndef RenderTarget_h
#define RenderTarget_h

#include <Vortex2D/Renderer/Common.h>
#include <Vortex2D/Renderer/Drawable.h>

namespace Vortex2D { namespace Renderer {

/**
 * @brief And interface to represent a target that can be rendered to.
 * This is implemented by the RenderWindow (in the examples) and the RenderTexture
 */
struct RenderTarget
{
    RenderTarget(float width, float height);

    virtual ~RenderTarget(){}

    glm::mat4 Orth;

    vk::Framebuffer Framebuffer;

    vk::RenderPass RenderPass;

    uint32_t Width, Height;
};

}}

#endif /* RenderTarget_h */
