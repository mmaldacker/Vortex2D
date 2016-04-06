//
//  Advection.cpp
//  Vertex2D
//
//  Created by Maximilian Maldacker on 08/09/2015.
//  Copyright (c) 2015 Maximilian Maldacker. All rights reserved.
//

#include "Advection.h"
#include "Disable.h"

namespace Fluid
{

Advection::Advection(Dimensions dimensions, float dt)
    : mDimensions(dimensions)
    , mVelocity(dimensions.Size, 2, true, true)
    , mDensity(dimensions.Size, 4, true, true)
    , mAdvect("TexturePosition.vsh", "Advect.fsh")
    , mAdvectDensity("TexturePosition.vsh", "AdvectDensity.fsh")
{
    mDensity.linear();

    mVelocity.clear();
    mDensity.clear();

    mAdvect.Use().Set("delta", dt).Set("u_texture", 0).Set("u_velocity", 1).Set("h", dimensions.Size).Unuse();
    mAdvectDensity.Use().Set("delta", dt).Set("u_texture", 0).Set("u_velocity", 1).Set("h", dimensions.Size).Unuse();
}

void Advection::RenderVelocity(const std::vector<Renderer::Drawable*> & objects)
{
    Renderer::Enable e(GL_STENCIL_TEST);
    glStencilFunc(GL_EQUAL, 0, 0xFF);
    glStencilMask(0x00);

    mVelocity.begin();
    for(auto object : objects)
    {
        object->Render(mVelocity.Orth*mDimensions.InvScale);
    }
    mVelocity.end();
}

void Advection::RenderDensity(const std::vector<Renderer::Drawable*> & objects)
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

void Advection::RenderMask(Boundaries & boundaries)
{
    // FIXME is this necessary?
    boundaries.RenderMask(mVelocity);
    mVelocity.swap();
    boundaries.RenderMask(mVelocity);
    mVelocity.swap();
    boundaries.RenderMask(mDensity);
    mDensity.swap();
    boundaries.RenderMask(mDensity);
    mDensity.swap();
}

void Advection::Advect(Fluid::Buffer & buffer)
{

}

void Advection::Advect()
{
    Renderer::Enable e(GL_STENCIL_TEST);
    glStencilFunc(GL_EQUAL, 0, 0xFF);
    glStencilMask(0x00);

    mDensity.swap();
    mDensity = mAdvectDensity(Back(mDensity), Back(mVelocity));
    mVelocity.swap();
    mVelocity = mAdvect(Back(mVelocity), Back(mVelocity));
}

}