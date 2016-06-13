//
//  LevelSet.cpp
//  Vertex2D
//
//  Created by Maximilian Maldacker on 05/04/2016.
//  Copyright (c) 2016 Maximilian Maldacker. All rights reserved.
//

#include "LevelSet.h"
#include "Disable.h"
#include "Engine.h"

namespace Fluid
{

const char * RedistanceFrag = GLSL(
    in vec2 v_texCoord;
    out vec4 out_color;

    uniform sampler2D u_texture;
    uniform float delta;
    uniform vec2 h; //FIXME use textureSize

    vec2 bilerp(sampler2D u_texture, vec2 xy)
    {
        vec4 ij;
        ij.xy = floor(xy);
        ij.zw = ij.xy + 1.0;
        vec2 f = xy - ij.xy;

        vec4 st = (ij + 0.5) / vec4(h,h);

        vec2 t11 = texture(u_texture, st.xy).xy;
        vec2 t21 = texture(u_texture, st.xw).xy;
        vec2 t12 = texture(u_texture, st.zy).xy;
        vec2 t22 = texture(u_texture, st.zw).xy;

        return mix(mix(t11,t21,f.y),mix(t12,t22,f.y),f.x);
    }

    void main()
    {
        float s = sign(texture(u_texture, v_texCoord).x);

        float wxp = textureOffset(u_texture, v_texCoord, ivec2(1,0)).x;
        float wxn = textureOffset(u_texture, v_texCoord, ivec2(-1,0)).x;
        float wyp = textureOffset(u_texture, v_texCoord, ivec2(0,1)).x;
        float wyn = textureOffset(u_texture, v_texCoord, ivec2(0,-1)).x;

        vec2 w = normalize(vec2(wxp-wxn, wyp-wyn));
        
        vec2 stepBackCoords = gl_FragCoord.xy - 0.5 - delta * s * w;
        
        out_color = vec4(bilerp(u_texture, stepBackCoords).x + delta * s, 0.0, 0.0, 0.0);
    }
);

const char * LevelSetMaskFrag = GLSL(
    in vec2 v_texCoord;
    out vec4 out_color;

    uniform sampler2D u_texture;

    void main()
    {
        float x = texture(u_texture, v_texCoord).x;

        if(x < 0.0)
        {
            out_color = vec4(1.0, 0.0, 0.0, 0.0);
        }
        else
        {
            out_color = vec4(0.0);
        }
    }
);

LevelSet::LevelSet(Dimensions dimensions, float dt)
    : mDimensions(dimensions)
    , mLevelSet(dimensions.Size, 1, true)
    , mRedistance(Renderer::Shader::TexturePositionVert, RedistanceFrag)
    , mLevelSetMask(Renderer::Shader::TexturePositionVert, LevelSetMaskFrag)
{
    mLevelSet.clear();
    mLevelSet.clamp_to_edge();
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
    // FIXME need to set mask
    Renderer::Enable e(GL_STENCIL_TEST);
    glStencilFunc(GL_EQUAL, 0, 0xFF);
    glStencilMask(0x00);
    
    for(int i = 0 ; i < 20 ; i++)
    {
        mLevelSet.swap();
        mLevelSet = mRedistance(Back(mLevelSet));
    }
}

Context LevelSet::GetBoundaries()
{
    return mLevelSetMask(mLevelSet);
}

void LevelSet::Advect(Fluid::Engine &engine)
{
    engine.Advect(mLevelSet);
}

}