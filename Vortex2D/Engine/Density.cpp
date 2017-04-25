//
//  Density.cpp
//  Vortex2D
//

#include "Density.h"

#include <glm/gtx/transform.hpp>

#include <Vortex2D/Engine/World.h>

namespace Vortex2D { namespace Fluid {

Density::Density(Dimensions dimensions)
    : mDimensions(dimensions)
    , mDensity(dimensions.Size, 4, true)
{
    mDensity.Clear(glm::vec4(0.0));
}

Density::~Density()
{
}

void Density::Render(Renderer::Drawable& object)
{
    mDensity.Render(object, mDimensions.InvScale);
}

void Density::Advect(World & world)
{
    world.Advect(mDensity);
}

void Density::Render(const Renderer::Device& device, Renderer::RenderTarget & target)
{
    /*
    auto & densitySprite = mDensity.Sprite();
    densitySprite.SetProgram(mProgram);
    densitySprite.Render(target, glm::scale(glm::vec3(mDimensions.Scale, mDimensions.Scale, 1.0))*transform);
    */
}

}}
