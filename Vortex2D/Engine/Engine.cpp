//
//  Engine.cpp
//  Vortex
//
//  Created by Maximilian Maldacker on 08/04/2014.
//
//

#include "Engine.h"
#include "Disable.h"
#include "LevelSet.h"
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

const char * BoundaryMaskFrag = GLSL(
    in vec2 v_texCoord;

    uniform sampler2D u_dirichlet;
    uniform sampler2D u_neumann;

    void main()
    {
        float x = texture(u_dirichlet, v_texCoord).x;
        float y = texture(u_neumann, v_texCoord).x;

        if(x < 1.0 && y < 1.0)
        {
            discard;
        }
    }
);

const char * WeightsFrag = GLSL(
    in vec2 v_texCoord;
    out vec4 out_color;

    uniform sampler2D u_dirichlet;
    uniform sampler2D u_neumann;
    uniform float delta;

    void main()
    {
        vec4 p;
        p.x = textureOffset(u_dirichlet, v_texCoord, ivec2(1,0)).x;
        p.y = textureOffset(u_dirichlet, v_texCoord, ivec2(-1,0)).x;
        p.z = textureOffset(u_dirichlet, v_texCoord, ivec2(0,1)).x;
        p.w = textureOffset(u_dirichlet, v_texCoord, ivec2(0,-1)).x;

        vec4 q;
        q.x = textureOffset(u_neumann, v_texCoord, ivec2(2,0)).x;
        q.y = textureOffset(u_neumann, v_texCoord, ivec2(-2,0)).x;
        q.z = textureOffset(u_neumann, v_texCoord, ivec2(0,2)).x;
        q.w = textureOffset(u_neumann, v_texCoord, ivec2(0,-2)).x;

        float dx = 1.0;
        out_color = delta * (1.0 - max(p,q)) / (dx*dx);
    }
);

const char * DiagonalsFrag = GLSL(
    in vec2 v_texCoord;
    out vec4 out_color;

    uniform sampler2D u_texture; // this is the obstacles
    uniform float delta;

    const vec4 q = vec4(1.0);

    void main()
    {
        vec4 p;
        p.x = 1.0 - textureOffset(u_texture, v_texCoord, ivec2(2,0)).x;
        p.y = 1.0 - textureOffset(u_texture, v_texCoord, ivec2(-2,0)).x;
        p.z = 1.0 - textureOffset(u_texture, v_texCoord, ivec2(0,2)).x;
        p.w = 1.0 - textureOffset(u_texture, v_texCoord, ivec2(0,-2)).x;

        float dx = 1.0;
        out_color = vec4(delta * dot(p,q) / (dx*dx),0.0, 0.0, 0.0);
    }
);

const char * AdvectFrag = GLSL(
    uniform sampler2D u_texture;
    uniform sampler2D u_velocity;
    uniform float delta;

    in vec2 v_texCoord;
    out vec4 out_color;

    vec2 cubic(vec2 f1, vec2 f2, vec2 f3, vec2 f4, float xd)
    {
        float xd2 = xd * xd;
        float xd3 = xd2 * xd;

        return f1*(-0.5*xd + xd2 - 0.5*xd3) + f2*(1.0 - 2.5*xd2 + 1.5*xd3) + f3*(0.5*xd + 2.0*xd2 - 1.5*xd3) + f4*(-0.5*xd2 + 0.5*xd3);
    }

    vec2 bilerp(sampler2D u_texture, vec2 xy)
    {
        vec2 ij0 = floor(xy) - 1.0;
        vec2 ij1 = ij0 + 1.0;
        vec2 ij2 = ij1 + 1.0;
        vec2 ij3 = ij2 + 1.0;

        vec2 f = xy - ij1.xy;

        vec2 h = textureSize(u_texture, 0);
        vec2 s0 = (ij0 + 0.5) / h;
        vec2 s1 = (ij1 + 0.5) / h;
        vec2 s2 = (ij2 + 0.5) / h;
        vec2 s3 = (ij3 + 0.5) / h;

        vec2 t00 = texture(u_texture, vec2(s0.x, s0.y)).xy;
        vec2 t10 = texture(u_texture, vec2(s1.x, s0.y)).xy;
        vec2 t20 = texture(u_texture, vec2(s2.x, s0.y)).xy;
        vec2 t30 = texture(u_texture, vec2(s3.x, s0.y)).xy;
        vec2 t01 = texture(u_texture, vec2(s0.x, s1.y)).xy;
        vec2 t11 = texture(u_texture, vec2(s1.x, s1.y)).xy;
        vec2 t21 = texture(u_texture, vec2(s2.x, s1.y)).xy;
        vec2 t31 = texture(u_texture, vec2(s3.x, s1.y)).xy;
        vec2 t02 = texture(u_texture, vec2(s0.x, s2.y)).xy;
        vec2 t12 = texture(u_texture, vec2(s1.x, s2.y)).xy;
        vec2 t22 = texture(u_texture, vec2(s2.x, s2.y)).xy;
        vec2 t32 = texture(u_texture, vec2(s3.x, s2.y)).xy;
        vec2 t03 = texture(u_texture, vec2(s0.x, s3.y)).xy;
        vec2 t13 = texture(u_texture, vec2(s1.x, s3.y)).xy;
        vec2 t23 = texture(u_texture, vec2(s2.x, s3.y)).xy;
        vec2 t33 = texture(u_texture, vec2(s3.x, s3.y)).xy;

        return cubic(
                    cubic(t00, t01, t02, t03, f.y),
                    cubic(t10, t11, t12, t13, f.y),
                    cubic(t20, t21, t22, t23, f.y),
                    cubic(t30, t31, t32, t33, f.y),
                    f.x
                );
    }

    void main(void)
    {
        vec2 pos = gl_FragCoord.xy - 0.5;

        vec2 stepBackCoords = pos - delta * texture(u_velocity, v_texCoord).xy;
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


Engine::Engine(Dimensions dimensions, LinearSolver * linearSolver, float dt)
    : mDimensions(dimensions)
    , mData(dimensions.Size)
    , mLinearSolver(linearSolver)
    , mVelocity(dimensions.Size, 2, true, true)
    , mDirichletBoundaries(dimensions.Size, 1)
    , mNeumannBoundaries(glm::vec2(2.0f)*dimensions.Size, 1)
    , mBoundariesVelocity(dimensions.Size, 2)
    , mExtrapolateMask(dimensions.Size, 1)
    , mDiv(Renderer::Shader::TexturePositionVert, DivFrag)
    , mProject(Renderer::Shader::TexturePositionVert, ProjectFrag)
    , mAdvect(Renderer::Shader::TexturePositionVert, AdvectFrag)
    , mExtrapolate(Renderer::Shader::TexturePositionVert, ExtrapolateFrag)
    , mIdentity(Renderer::Shader::TexturePositionVert, Renderer::Shader::TexturePositionFrag)
    , mWeights(Renderer::Shader::TexturePositionVert, WeightsFrag)
    , mDiagonals(Renderer::Shader::TexturePositionVert, DiagonalsFrag)
    , mBoundaryMask(Renderer::Shader::TexturePositionVert, BoundaryMaskFrag)
    , mSurface(dimensions.Size)
{
    mVelocity.clear();
    mDirichletBoundaries.clear();

    mNeumannBoundaries.clear();
    mNeumannBoundaries.linear();

    mBoundariesVelocity.clear();

    mProject.Use().Set("u_texture", 0).Set("u_pressure", 1).Set("u_obstacles", 2).Set("u_obstacles_velocity", 3).Set("delta", dt).Unuse();
    mDiv.Use().Set("u_texture", 0).Set("u_obstacles", 1).Set("u_obstacles_velocity", 2).Unuse();
    mAdvect.Use().Set("delta", dt).Set("u_texture", 0).Set("u_velocity", 1).Unuse();
    mExtrapolate.Use().Set("u_texture", 0).Set("u_velocity", 1).Unuse();
    mIdentity.Use().Set("u_texture", 0).Unuse();
    mWeights.Use().Set("u_dirichlet", 0).Set("u_neumann", 1).Set("delta", dt).Unuse();
    mDiagonals.Use().Set("u_texture", 0).Set("delta", dt).Unuse();
    mBoundaryMask.Use().Set("u_dirichlet", 0).Set("u_neumann", 1).Unuse();

    mSurface.Colour = glm::vec4{0.0f};
}

void Engine::Solve()
{
    mData.Pressure.clear();

    RenderMask(mData.Pressure);
    RenderMask(mData.Pressure.swap());

    RenderMask(mVelocity.swap());
    RenderMask(mVelocity.swap());

    Renderer::Enable e(GL_STENCIL_TEST);
    glStencilFunc(GL_EQUAL, 0, 0xFF);
    glStencilMask(0x00);

    mData.Pressure = mDiv(mVelocity, mNeumannBoundaries, mBoundariesVelocity);
    mData.Weights = mWeights(mDirichletBoundaries, mNeumannBoundaries);
    mData.Diagonal = mDiagonals(mNeumannBoundaries);

    mLinearSolver->Init(mData);
    mLinearSolver->Solve(mData);

    mVelocity.swap() = mProject(Back(mVelocity), mData.Pressure, mNeumannBoundaries, mBoundariesVelocity);

    glStencilFunc(GL_NOTEQUAL, 0, 0xFF);
    mVelocity.begin();
    mSurface.Render(mVelocity.Orth);
    mVelocity.end();

    Extrapolate();

    Advect(mVelocity);
}

void Engine::RenderDirichlet(const std::vector<Renderer::Drawable*> & objects)
{
    mDirichletBoundaries.begin();
    for(auto object : objects)
    {
        object->Render(mDirichletBoundaries.Orth*mDimensions.InvScale);
    }
    mDirichletBoundaries.end();
}

void Engine::RenderNeumann(const std::vector<Renderer::Drawable*> & objects)
{
    mNeumannBoundaries.begin();
    auto scaled = glm::scale(mNeumannBoundaries.Orth, glm::vec3(2.0f, 2.0f, 1.0f));
    for(auto object : objects)
    {
        object->Render(scaled*mDimensions.InvScale);
    }
    mNeumannBoundaries.end();
}

void Engine::RenderMask(Buffer & mask)
{
    Renderer::Enable e(GL_STENCIL_TEST);
    Renderer::DisableColorMask c;

    glStencilFunc(GL_ALWAYS, 1, 0xFF); // write 1 in stencil buffer
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE); // replace value with above
    glStencilMask(0xFF); // enable stencil writing

    // clear stencil buffer
    mask.begin();
    glClearStencil(0);
    glClear(GL_STENCIL_BUFFER_BIT);
    mask.end();

    mask = mBoundaryMask(mDirichletBoundaries, mNeumannBoundaries);

    glStencilMask(0x00); // disable stencil writing
}

void Engine::RenderVelocities(const std::vector<Renderer::Drawable*> & objects)
{
    mBoundariesVelocity.begin({0.0f, 0.0f, 0.0f, 0.0f});
    for(auto object : objects)
    {
        object->Render(mBoundariesVelocity.Orth*mDimensions.InvScale);
    }
    mBoundariesVelocity.end();
}

void Engine::RenderFluid(Fluid::LevelSet &levelSet)
{
    mDirichletBoundaries = levelSet.GetBoundaries();
}

void Engine::Clear()
{
    mDirichletBoundaries.clear();
    mNeumannBoundaries.clear();
    mBoundariesVelocity.clear();
}

void Engine::RenderForce(const std::vector<Renderer::Drawable*> & objects)
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

void Engine::Advect(Fluid::Buffer & buffer)
{
    Renderer::Enable e(GL_STENCIL_TEST);
    glStencilFunc(GL_EQUAL, 0, 0xFF);
    glStencilMask(0x00);

    buffer.swap() = mAdvect(Back(buffer), Back(mVelocity));
}

void Engine::Extrapolate()
{
}

}
