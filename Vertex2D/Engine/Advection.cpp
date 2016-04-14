//
//  Advection.cpp
//  Vertex2D
//
//  Created by Maximilian Maldacker on 08/09/2015.
//  Copyright (c) 2015 Maximilian Maldacker. All rights reserved.
//

#include "Advection.h"
#include "Boundaries.h"
#include "Disable.h"

namespace Fluid
{

Advection::Advection(Dimensions dimensions, float dt)
    : mDimensions(dimensions)
    , mVelocity(dimensions.Size, 2, true, true)
    , mAdvect("TexturePosition.vsh", "Advect.fsh")
    , mExtrapolate("TexturePosition.vsh", "Extrapolate.fsh")
    , mIdentity("TexturePosition.vsh", "TexturePosition.fsh")

{
    mVelocity.clear();

    mAdvect.Use().Set("delta", dt).Set("u_texture", 0).Set("u_velocity", 1).Unuse();
    mExtrapolate.Use().Set("u_texture", 0).Set("u_velocity", 1).Unuse();
    mIdentity.Use().Set("u_texture", 0).Unuse();
}

void Advection::Render(const std::vector<Renderer::Drawable*> & objects)
{
    Renderer::Enable e(GL_STENCIL_TEST);
    glStencilFunc(GL_EQUAL, 0, 0xFF);
    glStencilMask(0x00);

    Renderer::Enable b(GL_BLEND);
    Renderer::BlendState s(GL_FUNC_ADD, GL_ONE, GL_ONE);

    mVelocity.begin();
    for(auto object : objects)
    {
        object->Render(mVelocity.Orth*mDimensions.InvScale);
    }
    mVelocity.end();
}

void Advection::RenderMask(Boundaries & boundaries)
{
    boundaries.RenderMask(mVelocity.swap());
    boundaries.RenderMask(mVelocity.swap());
}

void Advection::Advect(Fluid::Buffer & buffer)
{
    Renderer::Enable e(GL_STENCIL_TEST);
    glStencilFunc(GL_EQUAL, 0, 0xFF);
    glStencilMask(0x00);

    buffer.swap() = mAdvect(Back(buffer), Back(mVelocity));
}

void Advection::Advect()
{
    Advect(mVelocity);
}

void Advection::Extrapolate(LevelSet & levelSet)
{
    mVelocity.swap() = mIdentity(Back(mVelocity));

    Renderer::Enable e(GL_STENCIL_TEST);
    glStencilMask(0x00);
    glStencilFunc(GL_EQUAL, 1, 0xFF);
    for(int i = 0 ; i < 100 ; i++)
    {
        mVelocity.swap() = mExtrapolate(levelSet.mLevelSet, Back(mVelocity));
    }
}

}