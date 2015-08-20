//
//  Emitter.cpp
//  Vortex
//
//  Created by Maximilian Maldacker on 28/04/2014.
//
//

#include "Emitter.h"
#include <glm/gtx/vector_angle.hpp>

namespace Fluid
{

Emitter::Emitter(float size) : mShape({size,size})
{
}

void Emitter::SetSize(float size)
{
    mShape.SetRectangle({size, size});
}

void Emitter::RenderVelocity(const glm::mat4 & ortho)
{
    auto velocity = glm::rotate(glm::vec2{Magnitude, 0.0f}, Transformable->Rotation());
    mShape.Colour = glm::vec4{velocity, 0.0f, 0.0f};
    mShape.Render(Transformable->GetTransform(ortho));
}

void Emitter::RenderDensity(const glm::mat4 & ortho)
{
    mShape.Colour = Density;
    mShape.Render(Transformable->GetTransform(ortho));
}

}
