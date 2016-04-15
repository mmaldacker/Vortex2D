//
//  Water.cpp
//  Vertex2D
//
//  Created by Maximilian Maldacker on 05/04/2016.
//  Copyright (c) 2016 Maximilian Maldacker. All rights reserved.
//

#include "Water.h"

namespace Fluid
{

const char * WaterFrag = GLSL(
    in vec2 v_texCoord;
    uniform sampler2D u_texture;
    uniform vec4 u_Colour;

    out vec4 out_color;

    void main()
    {
        float x = texture(u_texture, v_texCoord).x;
        if(x > 0.0)
        {
            out_color = u_Colour;
        }
        else
        {
            out_color = vec4(0.0);
        }
    }
);

Water::Water(Fluid::LevelSet & levelSet)
    : mProgram(Renderer::Shader::TexturePositionVert, WaterFrag)
    , mWaterTexture(levelSet.mLevelSet.texture())
    , mQuad({mWaterTexture.Width(), mWaterTexture.Height()})
    , mColourUniform(mProgram, "u_Colour")
{
}

void Water::Render(const glm::mat4 & ortho)
{
    mProgram.Use().SetMVP(GetTransform(ortho));
    mColourUniform.Set(Colour);
    mWaterTexture.Bind(0);
    mQuad.Render();
    mWaterTexture.Unbind();
    mProgram.Unuse();
}

}