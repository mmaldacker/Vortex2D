//
//  Drawable.h
//  Vortex
//

#ifndef Vortex_Drawable_h
#define Vortex_Drawable_h

#include <Vortex2D/Renderer/Common.h>

namespace Vortex2D { namespace Renderer {

struct RenderTarget;
class Device;

/**
 * @brief An interface to represent an object that can be rendered on a RenderTarget
 */
class Drawable
{
public:
    virtual ~Drawable(){}

    virtual void Render(const Device& device, RenderTarget & target) = 0;

protected:
    std::vector<vk::CommandBuffer> mCommandBuffers;
};

}}

#endif
