//
//  LevelSet.cpp
//  Vertex2D
//
//  Created by Maximilian Maldacker on 05/04/2016.
//  Copyright (c) 2016 Maximilian Maldacker. All rights reserved.
//

#include "LevelSet.h"
#include "Disable.h"
#include "Advection.h"

namespace Fluid
{

LevelSet::LevelSet(Dimensions dimensions, float dt)
    : mDimensions(dimensions)
    , mLevelSet(dimensions.Size, 1, true)
    , mRedistance("TexturePosition.vsh", "Redistance.fsh")
{
    mLevelSet.clear();
    mLevelSet.clamp_to_edge();
    //mLevelSet.linear();
    mRedistance.Use().Set("delta", dt).Set("u_texture", 0).Set("h", dimensions.Size).Unuse();
}

void LevelSet::Render(const std::vector<Renderer::Drawable*> & objects)
{
    mLevelSet.begin({-1.0f, 0.0f, 0.0f, 0.0f});
    for(auto && object : objects)
    {
        object->Render(mLevelSet.Orth*mDimensions.InvScale);
    }
    mLevelSet.end();
}

void LevelSet::Redistance()
{
    for(int i = 0 ; i < 10 ; i++)
    {
        mLevelSet.swap();
        mLevelSet = mRedistance(Back(mLevelSet));
    }
}

void LevelSet::Advect(Advection & advection)
{
    advection.Advect(mLevelSet);
}

Renderer::Texture & LevelSet::Texture()
{
    return mLevelSet.texture();
}

}