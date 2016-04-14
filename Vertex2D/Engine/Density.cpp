//
//  Density.cpp
//  Vertex2D
//
//  Created by Maximilian Maldacker on 05/04/2016.
//  Copyright (c) 2016 Maximilian Maldacker. All rights reserved.
//

#include "Density.h"
#include "Disable.h"
#include "Advection.h"
#include "Boundaries.h"

namespace Fluid
{

const char * AdvectShader = GLSL(
    uniform sampler2D u_texture;
    uniform sampler2D u_velocity;
    uniform float delta;

    in vec2 v_texCoord;
    out vec4 out_color;

    void main(void)
    {
        vec2 stepBackCoords = gl_FragCoord.xy - 0.5 - delta * texture(u_velocity, v_texCoord).xy;
        vec2 stepForwardCoords = stepBackCoords + delta * texture(u_velocity, stepBackCoords).xy;
        
        stepBackCoords = stepBackCoords + (stepBackCoords - stepForwardCoords) * 0.5;
        
        out_color = texture(u_texture, (stepBackCoords+0.5)/textureSize(u_texture, 0));
    }
);

Density::Density(Dimensions dimensions, float dt)
    : mDimensions(dimensions)
    , mDensity(dimensions.Size, 4, true, true)
    , mAdvectDensity(Renderer::Shader::TexturePositionVert, AdvectShader)
{
    mDensity.linear();
    mDensity.clear();
    mAdvectDensity.Use().Set("delta", dt).Set("u_texture", 0).Set("u_velocity", 1).Unuse();
}

void Density::Render(const std::vector<Renderer::Drawable*> & objects)
{
    mDensity.begin();
    for(auto object : objects)
    {
        object->Render(mDensity.Orth*mDimensions.InvScale);
    }
    mDensity.end();
}

void Density::Advect(Advection & advection)
{
    mDensity.swap() = mAdvectDensity(Back(mDensity), Back(advection.mVelocity));
}

Renderer::Sprite Density::Sprite()
{
    return {mDensity.texture()};
}

}