//
//  Water.cpp
//  Vertex2D
//
//  Created by Maximilian Maldacker on 05/04/2016.
//  Copyright (c) 2016 Maximilian Maldacker. All rights reserved.
//

#include "Water.h"

Water::Water(Fluid::LevelSet & levelSet)
    : mProgram("TexturePosition.vsh", "Water.fsh")
    , mWaterTexture(levelSet.Texture())
    , mQuad({mWaterTexture.Width(), mWaterTexture.Height()})
{
}

void Water::Render(const glm::mat4 & ortho)
{
    mProgram.Use().SetMVP(GetTransform(ortho));
    mWaterTexture.Bind(0);
    mQuad.Render();
    mWaterTexture.Unbind();
    mProgram.Unuse();
}