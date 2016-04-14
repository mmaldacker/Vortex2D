//
//  Engine.cpp
//  Vortex
//
//  Created by Maximilian Maldacker on 08/04/2014.
//
//

#include "Engine.h"
#include "Disable.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <iostream>

namespace Fluid
{

const char * DivFrag = GLSL(
    uniform sampler2D u_texture; // this is the velocity texture
    uniform sampler2D u_obstacles;
    uniform sampler2D u_obstacles_velocity;

    in vec2 v_texCoord;
    out vec4 out_color;

    void main()
    {
        vec2  uv  = texture(u_texture, v_texCoord).xy;
        float uxp = textureOffset(u_texture, v_texCoord, ivec2(1,0)).x;
        float uxn = uv.x;
        float vyp = textureOffset(u_texture, v_texCoord, ivec2(0,1)).y;
        float vyn = uv.y;

        float c   = 1.0 - texture(u_obstacles, v_texCoord).x;
        float cxp = 1.0 - textureOffset(u_obstacles, v_texCoord, ivec2(2,0)).x;
        float cxn = 1.0 - textureOffset(u_obstacles, v_texCoord, ivec2(-2,0)).x;
        float cyp = 1.0 - textureOffset(u_obstacles, v_texCoord, ivec2(0,2)).x;
        float cyn = 1.0 - textureOffset(u_obstacles, v_texCoord, ivec2(0,-2)).x;

        float solid_uxp = textureOffset(u_obstacles_velocity, v_texCoord, ivec2(1,0)).x;
        float solid_uxn = textureOffset(u_obstacles_velocity, v_texCoord, ivec2(-1,0)).x;
        float solid_vyp = textureOffset(u_obstacles_velocity, v_texCoord, ivec2(0,1)).y;
        float solid_vyn = textureOffset(u_obstacles_velocity, v_texCoord, ivec2(0,-1)).y;

        float dx = 1.0;
        float div = -(cxp * uxp - cxn * uxn + cyp * vyp - cyn * vyn) / dx;
        div += ((cxp-c)*solid_uxp - (cxn-c)*solid_uxn + (cyp-c)*solid_vyp - (cyn-c)*solid_vyn) / dx;
        
        //pressure, div, 0, 0
        out_color = vec4(0.0, div, 0.0, 0.0);
    }
);

const char * ProjectFrag = GLSL(
    in vec2 v_texCoord;
    out vec4 out_color;

    uniform sampler2D u_texture; // this is the velocity texture
    uniform sampler2D u_pressure;
    uniform sampler2D u_obstacles;
    uniform sampler2D u_obstacles_velocity;
    uniform float delta;

    void main()
    {

        vec2 cell = texture(u_texture, v_texCoord).xy;

        float pxp = textureOffset(u_pressure, v_texCoord, ivec2(1,0)).x;
        float pxn = textureOffset(u_pressure, v_texCoord, ivec2(-1,0)).x;
        float pyp = textureOffset(u_pressure, v_texCoord, ivec2(0,1)).x;
        float pyn = textureOffset(u_pressure, v_texCoord, ivec2(0,-1)).x;

        float cxp = textureOffset(u_obstacles, v_texCoord, ivec2(2,0)).x;
        float cxn = textureOffset(u_obstacles, v_texCoord, ivec2(-2,0)).x;
        float cyp = textureOffset(u_obstacles, v_texCoord, ivec2(0,2)).x;
        float cyn = textureOffset(u_obstacles, v_texCoord, ivec2(0,-2)).x;

        vec2 pGrad = vec2(pxp-pxn, pyp-pyn);

        vec2 mask = vec2(1.0);
        vec2 obsV = vec2(0.0);

        if (cxp > 0.0)
        {
            mask.x = 0.0;
            obsV.x = textureOffset(u_obstacles_velocity, v_texCoord, ivec2(1,0)).x;
        }
        if (cxn > 0.0)
        {
            mask.x = 0.0;
            obsV.x = textureOffset(u_obstacles_velocity, v_texCoord, ivec2(-1,0)).x;
        }
        if (cyp > 0.0)
        {
            mask.y = 0.0;
            obsV.y = textureOffset(u_obstacles_velocity, v_texCoord, ivec2(0,1)).y;
        }
        if (cyn > 0.0)
        {
            mask.y = 0.0;
            obsV.y = textureOffset(u_obstacles_velocity, v_texCoord, ivec2(0,-1)).y;
        }
        
        float dx = 1.0;
        vec2 new_cell = cell - delta * pGrad / dx;
        out_color = vec4(mask * new_cell + obsV, 0.0, 0.0);
    }
);

Engine::Engine(Dimensions dimensions, Boundaries & boundaries, Advection & advection, LinearSolver * linearSolver, float dt)
    : mDimensions(dimensions)
    , mData(dimensions.Size)
    , mBoundaries(boundaries)
    , mAdvection(advection)
    , mLinearSolver(linearSolver)
    , mDiv(Renderer::Shader::TexturePositionVert, DivFrag)
    , mProject(Renderer::Shader::TexturePositionVert, ProjectFrag)
    , mSurface(dimensions.Size)
{
    mProject.Use().Set("u_texture", 0).Set("u_pressure", 1).Set("u_obstacles", 2).Set("u_obstacles_velocity", 3).Set("delta", dt).Unuse();
    mDiv.Use().Set("u_texture", 0).Set("u_obstacles", 1).Set("u_obstacles_velocity", 2).Unuse();
    mSurface.Colour = glm::vec4{0.0f};
}

void Engine::Solve()
{
    mData.Pressure.clear();

    mBoundaries.RenderMask(mData.Pressure);
    mBoundaries.RenderMask(mData.Pressure.swap());

    Renderer::Enable e(GL_STENCIL_TEST);
    glStencilFunc(GL_EQUAL, 0, 0xFF);
    glStencilMask(0x00);

    mData.Pressure = mDiv(mAdvection.mVelocity, mBoundaries.mNeumannBoundaries, mBoundaries.mBoundariesVelocity);
    mData.Weights = mBoundaries.GetWeights();
    mData.Diagonal = mBoundaries.GetDiagonals();

    mLinearSolver->Init(mData);
    mLinearSolver->Solve(mData);

    mAdvection.mVelocity.swap() = mProject(Back(mAdvection.mVelocity), mData.Pressure, mBoundaries.mNeumannBoundaries, mBoundaries.mBoundariesVelocity);

    glStencilFunc(GL_NOTEQUAL, 0, 0xFF);
    mAdvection.mVelocity.begin();
    mSurface.Render(mAdvection.mVelocity.Orth);
    mAdvection.mVelocity.end();
}

}
