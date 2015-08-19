//
//  Obstacle.h
//  Vortex
//
//  Created by Maximilian Maldacker on 07/05/2014.
//
//

#ifndef __Vortex__Obstacle__
#define __Vortex__Obstacle__

#include "FluidInput.h"
#include "Shapes.h"

namespace Fluid
{

class Obstacle : public FluidInput
{
public:
    void Set(const Renderer::Path & path);
    void RenderObstacle(const glm::mat4 & ortho);
    void RenderObstacleVelocity(const glm::mat4 & ortho);

private:
    Renderer::Shape mPolygon;
};

}

#endif /* defined(__Vortex__Obstacle__) */
