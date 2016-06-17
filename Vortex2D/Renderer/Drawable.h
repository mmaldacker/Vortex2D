//
//  Drawable.h
//  Vortex
//
//  Created by Maximilian Maldacker on 29/04/2014.
//
//

#ifndef Vortex_Drawable_h
#define Vortex_Drawable_h

#include "Common.h"

namespace Vortex2D { namespace Renderer {

struct RenderTarget;

/**
 * @brief An interface to represent an object that can be rendered on a RenderTarget
 */
struct Drawable
{
    virtual ~Drawable(){}

    /**
     * @brief Renders on a RenderTarget
     * @param target the target (RenderTexture or RenderWindow)
     * @param transform an optional transform to apply to the object
     */
    virtual void Render(RenderTarget & target, const glm::mat4 & transform = glm::mat4()) = 0;
};

}}

#endif
