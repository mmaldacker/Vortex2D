//
//  FluidInput.h
//  Vortex
//
//  Created by Maximilian Maldacker on 28/04/2014.
//
//

#ifndef __Vortex__FluidInput__
#define __Vortex__FluidInput__

#include "Common.h"
#include "Transformable.h"

namespace Fluid
{

struct FluidInput
{
    virtual void RenderObstacle(const glm::mat4 & ortho) {}
    virtual void RenderObstacleVelocity(const glm::mat4 & ortho) {}
    virtual void RenderVelocity(const glm::mat4 & ortho) {}
    virtual void RenderDensity(const glm::mat4 & ortho) {}
    Renderer::Transformable * Transformable;
};

}

#endif /* defined(__Vortex__FluidInput__) */
