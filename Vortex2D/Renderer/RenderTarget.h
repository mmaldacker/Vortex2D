//
//  RenderTarget.h
//  Vortex2D
//
//  Created by Maximilian Maldacker on 15/06/2016.
//
//

#ifndef RenderTarget_h
#define RenderTarget_h

#include "Common.h"
#include "Drawable.h"

namespace Renderer
{

struct RenderTarget
{
    RenderTarget(float width, float height)
        : Orth(glm::ortho(0.0f, width, 0.0f, height))
    {}

    virtual void Clear(const glm::vec4 & colour) = 0;
    virtual void Render(const DrawablesVector & objects, const glm::mat4 & transform) = 0;

    glm::mat4 Orth;
};

}

#endif /* RenderTarget_h */
