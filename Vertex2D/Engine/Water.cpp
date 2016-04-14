//
//  Water.cpp
//  Vertex2D
//
//  Created by Maximilian Maldacker on 05/04/2016.
//  Copyright (c) 2016 Maximilian Maldacker. All rights reserved.
//

#include "Water.h"

const char * WaterFrag = GLSL(
    in vec2 v_texCoord;
    uniform sampler2D u_texture;

    out vec4 out_color;

    void main()
    {
        float x = texture(u_texture, v_texCoord).x;
        if(x > 0.0)
        {
            out_color = vec4(0.0, 1.0, 0.0, 1.0);
        }
        else
        {
            out_color = vec4(0.0);
        }
    }
);

Water::Water(Fluid::LevelSet & levelSet)
: mProgram(Renderer::Shader::TexturePositionVert, WaterFrag)
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