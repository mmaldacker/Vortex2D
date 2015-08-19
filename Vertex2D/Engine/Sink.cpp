//
//  Sink.cpp
//  Vortex
//
//  Created by Maximilian Maldacker on 08/05/2014.
//
//

#include "Sink.h"
#include <glm/gtx/vector_angle.hpp>


namespace Fluid
{

Sink::Sink(float size)
{
    SetSize(size);
}

void Sink::SetSize(float size)
{
    mShape.SetRectangle({size,size*0.25f});
    mShape.SetAnchor(glm::vec2{size*0.5f, size*0.125f});
}

void Sink::RenderObstacleVelocity(const glm::mat4 & ortho)
{
    auto velocity = glm::rotate(glm::vec2{0.0f, -Magnitude}, Transformable->Rotation());
    mShape.Colour = glm::vec4{velocity, 0.0f, 0.0f};
    mShape.Render(Transformable->GetTransform(ortho));
}

}
