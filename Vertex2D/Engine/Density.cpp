//
//  Density.cpp
//  Vertex2D
//
//  Created by Maximilian Maldacker on 05/04/2016.
//  Copyright (c) 2016 Maximilian Maldacker. All rights reserved.
//

#include "Density.h"
#include "Disable.h"
#include "Advection.h"
#include "Boundaries.h"

namespace Fluid
{

Density::Density(Dimensions dimensions, float dt)
    : mDimensions(dimensions)
    , mDensity(dimensions.Size, 4, true, true)
    , mAdvectDensity("TexturePosition.vsh", "AdvectDensity.fsh")
{
    mDensity.linear();
    mDensity.clear();
    mAdvectDensity.Use().Set("delta", dt).Set("u_texture", 0).Set("u_velocity", 1).Set("h", dimensions.Size).Unuse();
}

void Density::Render(const std::vector<Renderer::Drawable*> & objects)
{
    Renderer::Enable e(GL_STENCIL_TEST);
    glStencilFunc(GL_EQUAL, 0, 0xFF);
    glStencilMask(0x00);

    mDensity.begin();
    for(auto object : objects)
    {
        object->Render(mDensity.Orth*mDimensions.InvScale);
    }
    mDensity.end();
}

void Density::RenderMask(Boundaries & boundaries)
{
    // FIXME is this necessary?
    boundaries.RenderMask(mDensity.swap());
    boundaries.RenderMask(mDensity.swap());
}

void Density::Advect(Advection & advection)
{
    mDensity.swap() = mAdvectDensity(Back(mDensity), Back(advection.mVelocity));
}

Renderer::Sprite Density::Sprite()
{
    return {mDensity.texture()};
}

}