//
//  Water.cpp
//  Vertex2D
//
//  Created by Maximilian Maldacker on 05/04/2016.
//  Copyright (c) 2016 Maximilian Maldacker. All rights reserved.
//

#include "Water.h"
#include "Shader.h"
#include "Disable.h"
#include "Engine.h"

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

const char * RedistanceFrag = GLSL(
   in vec2 v_texCoord;
   out vec4 out_color;

   uniform sampler2D u_texture;
   uniform sampler2D u_sign;
   uniform float delta;

   float g(float s, float wxy, float wxp, float wxn, float wyp, float wyn)
   {
       float a = wxy - wxn;
       float b = wxp - wxy;
       float c = wxy - wyn;
       float d = wyp - wxy;

       if(s > 0)
       {
           float ap = max(a,0);
           float bn = min(b,0);
           float cp = max(c,0);
           float dn = min(d,0);

           return sqrt(max(ap*ap, bn*bn) + max(cp*cp, dn*dn)) - 1.0;
       }
       else
       {
           float an = min(a,0);
           float bp = max(b,0);
           float cn = min(c,0);
           float dp = max(d,0);

           return sqrt(max(an*an, bp*bp) + max(cn*cn, dp*dp)) - 1.0;
       }

    }

   void main()
   {
       float s = texture(u_sign, v_texCoord).x;

       float wxy = texture(u_texture, v_texCoord).x;
       float wxp = textureOffset(u_texture, v_texCoord, ivec2(1,0)).x;
       float wxn = textureOffset(u_texture, v_texCoord, ivec2(-1,0)).x;
       float wyp = textureOffset(u_texture, v_texCoord, ivec2(0,1)).x;
       float wyn = textureOffset(u_texture, v_texCoord, ivec2(0,-1)).x;

       out_color = vec4(wxy - delta * s * g(s, wxy, wxp, wxn, wyp, wyn), 0.0, 0.0, 0.0);

   }
);

const char * SignLevelSetMaskFrag = GLSL(
    in vec2 v_texCoord;
    out vec4 out_color;

    uniform sampler2D u_texture;

    void main()
    {
        float x = texture(u_texture, v_texCoord).x;
        float s = x / sqrt(x*x + 1);
        out_color = vec4(s, 0.0, 0.0, 0.0);
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

Water::Water(Dimensions dimensions)
    : mDimensions(dimensions)
    , mLevelSet(dimensions.Size, 1, true)
    , mSignLevelSet(dimensions.Size, 1)
    , mRedistance(Renderer::Shader::TexturePositionVert, RedistanceFrag)
    , mSign(Renderer::Shader::TexturePositionVert, SignLevelSetMaskFrag)
    , mLevelSetMask(Renderer::Shader::TexturePositionVert, LevelSetMaskFrag)
    , mRenderProgram(Renderer::Shader::TexturePositionVert, WaterFrag)
    , mColourUniform(mRenderProgram, "u_Colour")
{
    mLevelSet.ClampToEdge();
    mLevelSet.Linear();
    Clear();
    mRedistance.Use().Set("delta", 0.1f).Set("u_texture", 0).Set("u_sign", 1).Unuse();
    mSign.Use().Set("u_texture", 0).Unuse();
    mRenderProgram.Use().Set("u_texture", 0).Unuse();
}

void Water::Render(Renderer::RenderTarget & target, const glm::mat4 & transform)
{
    auto & levelSetSprite = mLevelSet.Sprite();
    levelSetSprite.SetProgram(mRenderProgram);
    mRenderProgram.Use();
    mColourUniform.Set(Colour);
    levelSetSprite.Render(target, glm::scale(glm::vec3(mDimensions.Scale, mDimensions.Scale, 1.0))*transform);
}

void Water::Render(Renderer::Drawable & object)
{
    Renderer::Disable d(GL_BLEND);
    mLevelSet.Render(object, mDimensions.InvScale);
    Redistance(true);
}

void Water::Clear()
{
    mLevelSet.Clear({-1.0f, 0.0f, 0.0f, 0.0f});
}

void Water::Redistance(bool reinitialize)
{
    mSignLevelSet = mSign(mLevelSet);

    int num_iterations = reinitialize ? 20 : 2;
    for(int i = 0 ; i < num_iterations ; i++)
    {
        mLevelSet.Swap() = mRedistance(Back(mLevelSet), mSignLevelSet);
    }
}

Renderer::Sprite & Water::GetBoundaries()
{
    auto & levelSetSprite = mLevelSet.Sprite();
    levelSetSprite.SetProgram(mLevelSetMask);
    return levelSetSprite;
}

void Water::Advect(Fluid::Engine &engine)
{
    engine.Advect(mLevelSet);
    Redistance();
}


}