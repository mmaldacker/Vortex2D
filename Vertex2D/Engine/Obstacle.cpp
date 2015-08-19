//
//  Obstacle.cpp
//  Vortex
//
//  Created by Maximilian Maldacker on 07/05/2014.
//
//

#include "Obstacle.h"

namespace Fluid
{

void Obstacle::Set(const Renderer::Path & path)
{
    mPolygon.SetType(GL_TRIANGLES);
    mPolygon.Set(path);
    mPolygon.Colour = {1,1,1,1};
}

void Obstacle::RenderObstacle(const glm::mat4 & ortho)
{
    mPolygon.Render(Transformable->GetTransform(ortho));
}

void Obstacle::RenderObstacleVelocity(const glm::mat4 & ortho)
{

}

}