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

const char * AdvectFrag = GLSL(
    uniform sampler2D u_texture;
    uniform sampler2D u_velocity;
    uniform float delta;

    in vec2 v_texCoord;
    out vec4 out_color;

    vec2 bilerp(sampler2D u_texture, vec2 xy)
    {
        vec4 ij;
        ij.xy = floor(xy);
        ij.zw = ij.xy + 1.0;
        vec2 f = xy - ij.xy;

        vec2 h = textureSize(u_texture, 0);
        vec4 st = (ij + vec4(0.5)) / vec4(h,h);

        vec2 t11 = texture(u_texture, st.xy).xy;
        vec2 t21 = texture(u_texture, st.xw).xy;
        vec2 t12 = texture(u_texture, st.zy).xy;
        vec2 t22 = texture(u_texture, st.zw).xy;

        return mix(mix(t11,t21,f.y),mix(t12,t22,f.y),f.x);
    }

    void main(void)
    {
        vec2 stepBackCoords = gl_FragCoord.xy - 0.5 - delta * texture(u_velocity, v_texCoord).xy;
        vec2 stepForwardCoords = stepBackCoords + delta * bilerp(u_velocity, stepBackCoords);

        stepBackCoords = stepBackCoords + (stepBackCoords - stepForwardCoords) * 0.5;
        
        out_color = vec4(bilerp(u_texture, stepBackCoords), 0.0, 0.0);
    }
);

const char * ExtrapolateFrag = GLSL(
    in vec2 v_texCoord;
    out vec4 out_color;

    uniform sampler2D u_texture;
    uniform sampler2D u_velocity;

    void main()
    {
        float s = sign(texture(u_texture, v_texCoord).x);

        float wxp = textureOffset(u_texture, v_texCoord, ivec2(1,0)).x;
        float wxn = textureOffset(u_texture, v_texCoord, ivec2(-1,0)).x;
        float wyp = textureOffset(u_texture, v_texCoord, ivec2(0,1)).x;
        float wyn = textureOffset(u_texture, v_texCoord, ivec2(0,-1)).x;

        vec2 w = normalize(vec2(wxp-wxn, wyp-wyn));

        float dx = 1.0 / textureSize(u_texture, 0).x;

        vec2 coords = gl_FragCoord.xy - 0.5 - s * w;

        vec2 stepBackwardsCoords = (coords + 0.5) * dx;
        
        out_color = vec4(texture(u_velocity, stepBackwardsCoords).xy, 0.0, 0.0);
    }
);

Advection::Advection(Dimensions dimensions, float dt)
    : mDimensions(dimensions)
    , mVelocity(dimensions.Size, 2, true, true)
    , mAdvect(Renderer::Shader::TexturePositionVert, AdvectFrag)
    , mExtrapolate(Renderer::Shader::TexturePositionVert, ExtrapolateFrag)
    , mIdentity(Renderer::Shader::TexturePositionVert, Renderer::Shader::TexturePositionFrag)
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