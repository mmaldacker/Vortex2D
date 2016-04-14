//
//  Boundaries.cpp
//  Vertex2D
//
//  Created by Maximilian Maldacker on 01/09/2015.
//  Copyright (c) 2015 Maximilian Maldacker. All rights reserved.
//

#include "Boundaries.h"
#include "Disable.h"

namespace Fluid
{

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

Boundaries::Boundaries(Dimensions dimensions, float dt)
    : mDimensions(dimensions)
    , mDirichletBoundaries(dimensions.Size, 1)
    , mNeumannBoundaries(glm::vec2(2.0f)*dimensions.Size, 1)
    , mBoundariesVelocity(dimensions.Size, 2)
    , mWeights(Renderer::Shader::TexturePositionVert, WeightsFrag)
    , mDiagonals(Renderer::Shader::TexturePositionVert, DiagonalsFrag)
    , mBoundaryMask(Renderer::Shader::TexturePositionVert, BoundaryMaskFrag)
    , mLevelSetMask(Renderer::Shader::TexturePositionVert, LevelSetMaskFrag)
{
    mDirichletBoundaries.clear();

    mNeumannBoundaries.clear();
    mNeumannBoundaries.linear();

    mBoundariesVelocity.clear();

    mWeights.Use().Set("u_dirichlet", 0).Set("u_neumann", 1).Set("delta", dt).Unuse();
    mDiagonals.Use().Set("u_texture", 0).Set("delta", dt).Unuse();
    mBoundaryMask.Use().Set("u_dirichlet", 0).Set("u_neumann", 1).Unuse();
}

void Boundaries::RenderDirichlet(const std::vector<Renderer::Drawable*> & objects)
{
    mDirichletBoundaries.begin();
    for(auto object : objects)
    {
        object->Render(mDirichletBoundaries.Orth*mDimensions.InvScale);
    }
    mDirichletBoundaries.end();
}

void Boundaries::RenderNeumann(const std::vector<Renderer::Drawable*> & objects)
{
    mNeumannBoundaries.begin();
    auto scaled = glm::scale(mNeumannBoundaries.Orth, glm::vec3(2.0f, 2.0f, 1.0f));
    for(auto object : objects)
    {
        object->Render(scaled*mDimensions.InvScale);
    }
    mNeumannBoundaries.end();
}

void Boundaries::RenderMask(Buffer & mask)
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

void Boundaries::RenderVelocities(const std::vector<Renderer::Drawable*> & objects)
{
    mBoundariesVelocity.begin({0.0f, 0.0f, 0.0f, 0.0f});
    for(auto object : objects)
    {
        object->Render(mBoundariesVelocity.Orth*mDimensions.InvScale);
    }
    mBoundariesVelocity.end();
}

void Boundaries::RenderFluid(Fluid::LevelSet &levelSet)
{
    mDirichletBoundaries = mLevelSetMask(levelSet.mLevelSet);
}

void Boundaries::Clear()
{
    mDirichletBoundaries.clear();
    mNeumannBoundaries.clear();
    mBoundariesVelocity.clear();
}

}