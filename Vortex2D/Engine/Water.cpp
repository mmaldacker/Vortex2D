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
   uniform float delta;

   float maxmod(float a, float b)
   {
       if(abs(a) > abs(b))
       {
           return a;
       }
       else
       {
           return b;
       }
   }

   float difference(float w, float p, float n)
   {
       if(sign(p - w) == sign(w - n))
       {
           return 0.5*(p-n);
       }
       else
       {
           return maxmod(p - w, w - n);
       }
   }

   vec2 bilerp(sampler2D u_texture, vec2 xy)
   {
       vec4 ij;
       ij.xy = floor(xy);
       ij.zw = ij.xy + 1.0;
       vec2 f = xy - ij.xy;

       vec2 h = textureSize(u_texture, 0);
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

       float wxy = texture(u_texture, v_texCoord).x;
       float wxp = textureOffset(u_texture, v_texCoord, ivec2(1,0)).x;
       float wxn = textureOffset(u_texture, v_texCoord, ivec2(-1,0)).x;
       float wyp = textureOffset(u_texture, v_texCoord, ivec2(0,1)).x;
       float wyn = textureOffset(u_texture, v_texCoord, ivec2(0,-1)).x;
       
       vec2 w = normalize(vec2(difference(wxy, wxp, wxn), difference(wxy, wyp, wyn)));
       
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

Water::Water(Dimensions dimensions, float dt)
    : mDimensions(dimensions)
    , mLevelSet(dimensions.Size, 1, true)
    , mRedistance(Renderer::Shader::TexturePositionVert, RedistanceFrag)
    , mLevelSetMask(Renderer::Shader::TexturePositionVert, LevelSetMaskFrag)
    , mProgram(Renderer::Shader::TexturePositionVert, WaterFrag)
    , mColourUniform(mProgram, "u_Colour")
{
    mLevelSet.Clear(glm::vec4(0.0));
    mLevelSet.ClampToEdge();
    mRedistance.Use().Set("delta", dt).Set("u_texture", 0).Unuse();
    mProgram.Use().Set("u_texture", 0).Unuse();
}

void Water::Render(Renderer::RenderTarget & target, const glm::mat4 & transform)
{
    auto & levelSetSprite = mLevelSet.Sprite();
    levelSetSprite.SetProgram(mProgram);
    mProgram.Use();
    mColourUniform.Set(Colour);
    levelSetSprite.Render(target, transform*glm::scale(glm::vec3(mDimensions.Scale, mDimensions.Scale, 1.0)));
}

void Water::Render(Renderer::Drawable & object)
{
    mLevelSet.Clear({-1.0f, 0.0f, 0.0f, 0.0f});
    mLevelSet.Render(object, mDimensions.InvScale);
}

void Water::Redistance()
{
    // FIXME need to set mask
    Renderer::Enable e(GL_STENCIL_TEST);
    glStencilFunc(GL_EQUAL, 0, 0xFF);
    glStencilMask(0x00);

    for(int i = 0 ; i < 20 ; i++)
    {
        mLevelSet.Swap() = mRedistance(Back(mLevelSet));
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
    // FIXME need to set mask
    engine.Advect(mLevelSet);
}


}