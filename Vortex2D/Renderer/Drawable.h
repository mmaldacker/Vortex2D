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

namespace Renderer
{

struct RenderTarget;

struct Drawable
{
    virtual ~Drawable(){}
    virtual void Render(RenderTarget & target, const glm::mat4 & transform = glm::mat4()) = 0;
};

}

#endif
