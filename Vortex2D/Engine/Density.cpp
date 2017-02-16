//
//  Density.cpp
//  Vortex2D
//

#include "Density.h"

#include <Vortex2D/Renderer/Disable.h>

#include <Vortex2D/Engine/World.h>

namespace Vortex2D { namespace Fluid {

Density::Density(Dimensions dimensions)
    : mDimensions(dimensions)
    , mDensity(dimensions.Size, 4, true)
    , mProgram(Renderer::Shader::TexturePositionVert, Renderer::Shader::TexturePositionFrag)
{
    mDensity.Clear(glm::vec4(0.0));
    mProgram.Use().Set("u_texture", 0).Unuse();
}

Density::~Density()
{
}

void Density::Render(Renderer::Drawable& object)
{
    Renderer::Disable d(GL_BLEND);
    mDensity.Render(object, mDimensions.InvScale);
}

void Density::Advect(World & world)
{
    world.Advect(mDensity);
}

void Density::Render(Renderer::RenderTarget& target, const glm::mat4& transform)
{
    auto & densitySprite = mDensity.Sprite();
    densitySprite.SetProgram(mProgram);
    densitySprite.Render(target, glm::scale(glm::vec3(mDimensions.Scale, mDimensions.Scale, 1.0))*transform);
}

}}
