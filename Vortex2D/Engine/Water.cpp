//
//  Water.cpp
//  Vortex2D
//

#include "Water.h"
#include "Shader.h"
#include "Disable.h"
#include "Engine.h"

namespace Vortex2D { namespace Fluid {

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

   uniform sampler2D u_levelSet;
   uniform sampler2D u_levelSet0;
   uniform float delta;

   float g(float s, float w, float wxp, float wxn, float wyp, float wyn)
   {
       float a = w - wxn;
       float b = wxp - w;
       float c = w - wyn;
       float d = wyp - w;

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
       float w0 = texture(u_levelSet0, v_texCoord).x;
       float wxp0 = textureOffset(u_levelSet0, v_texCoord, ivec2(1,0)).x;
       float wxn0 = textureOffset(u_levelSet0, v_texCoord, ivec2(-1,0)).x;
       float wyp0 = textureOffset(u_levelSet0, v_texCoord, ivec2(0,1)).x;
       float wyn0 = textureOffset(u_levelSet0, v_texCoord, ivec2(0,-1)).x;

       float w = texture(u_levelSet, v_texCoord).x;
       float wxp = textureOffset(u_levelSet, v_texCoord, ivec2(1,0)).x;
       float wxn = textureOffset(u_levelSet, v_texCoord, ivec2(-1,0)).x;
       float wyp = textureOffset(u_levelSet, v_texCoord, ivec2(0,1)).x;
       float wyn = textureOffset(u_levelSet, v_texCoord, ivec2(0,-1)).x;

       float s = sign(w0);

       if(w0*wxp0 < 0.0 || w0*wxn0 < 0.0 || w0*wyp0 < 0.0 || w0*wyn0 < 0.0)
       {
           float wx0 = wxp0 - wxn0;
           float wy0 = wyp0 - wyn0;
           float d = 2*w0 / sqrt(wx0*wx0 + wy0*wy0);
           out_color = vec4(w - delta * (s * abs(w) - d), 0.0, 0.0, 0.0);
       }
       else
       {
           out_color = vec4(w - delta * s * g(s, w, wxp, wxn, wyp, wyn), 0.0, 0.0, 0.0);
       }

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
             discard;
         }
     }
);

Water::Water(Dimensions dimensions)
    : mDimensions(dimensions)
    , mLevelSet(dimensions.Size, 1, true)
    , mLevelSet0(dimensions.Size, 1)
    , mRedistance(Renderer::Shader::TexturePositionVert, RedistanceFrag)
    , mIdentity(Renderer::Shader::TexturePositionVert, Renderer::Shader::TexturePositionFrag)
    , mLevelSetMask(Renderer::Shader::TexturePositionVert, LevelSetMaskFrag)
    , mRenderProgram(Renderer::Shader::TexturePositionVert, WaterFrag)
    , mColourUniform(mRenderProgram, "u_Colour")
{
    mLevelSet.ClampToEdge();
    mLevelSet.Linear();
    Clear();
    mRedistance.Use().Set("delta", 0.1f).Set("u_levelSet", 0).Set("u_levelSet0", 1).Unuse();
    mIdentity.Use().Set("u_texture", 0).Unuse();
    mRenderProgram.Use().Set("u_texture", 0).Unuse();
    mLevelSetMask.Use().Set("u_texture", 0).Unuse();
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
    Renderer::Disable d(GL_BLEND);

    mLevelSet0 = mIdentity(mLevelSet);

    int num_iterations = reinitialize ? 100 : 1;
    for(int i = 0 ; i < num_iterations ; i++)
    {
        mLevelSet.Swap() = mRedistance(Back(mLevelSet), mLevelSet0);
    }
}

void Water::RenderBoundaries(Vortex2D::Fluid::Engine &engine)
{
    Renderer::Disable d(GL_BLEND);

    auto & levelSetSprite = mLevelSet.Sprite();
    levelSetSprite.SetProgram(mLevelSetMask);
    engine.mDirichletBoundaries.Render(levelSetSprite);
}

void Water::Advect(Fluid::Engine &engine)
{
    engine.Advect(mLevelSet);
    Redistance();
}


}}
