//
//  Multigrid.cpp
//  Vertex2D
//
//  Created by Maximilian Maldacker on 10/09/2015.
//  Copyright (c) 2015 Maximilian Maldacker. All rights reserved.
//

#include "Multigrid.h"
#include "Disable.h"

namespace Fluid
{

int depth = 3;
int jacobi = 10;

Multigrid::Multigrid(Dimensions dimensions)
    : mCorrectShader("Diff.vsh", "Correct.fsh")
    , mDampedJacobiShader("Diff.vsh", "DampedJacobi.fsh")
    , mProlongateShader("Diff.vsh", "Prolongate.fsh")
    , mResidualShader("Diff.vsh", "Residual.fsh")
    , mRestrictShader("Restrict.vsh", "Restrict.fsh")
{
    auto size = dimensions.Size;
    
    for(int i = 0 ; i < depth ; ++i)
    {
        mQuads.emplace_back(size);
        mXs.emplace_back(size.x, size.y, Renderer::Texture::PixelFormat::RGF, Renderer::RenderTexture::DepthFormat::DEPTH24_STENCIL8);
        mXs.back().Clear();
        mWeightss.emplace_back(size.x, size.y, Renderer::Texture::PixelFormat::RGBAF);
        mWeightss.back().SetAliasTexParameters();
        mWeightss.back().Clear();

        //size = ((size - glm::vec2(2.0)) / glm::vec2(2.0f)) + glm::vec2(2.0f);
        size /= glm::vec2(2.0);
    }

    mCorrectShader.Use()
    .Set("u_texture", 0)
    .Set("u_residual", 1)
    .Unuse();

    mDampedJacobiShader.Use()
    .Set("u_texture", 0)
    .Set("u_weights", 1)
    .Unuse();

    mProlongateShader.Use()
    .Set("u_texture", 0)
    .Set("u_pressure", 1)
    .Unuse();

    mResidualShader.Use()
    .Set("u_texture", 0)
    .Set("u_weights", 1)
    .Unuse();

    mRestrictShader.Use()
    .Set("u_texture", 0)
    .Unuse();
}

Renderer::Reader Multigrid::GetPressureReader(int depth)
{
    return {mXs[depth].Front};
}

void Multigrid::Init(Boundaries & boundaries)
{
    float scale = 1.0f;
    for(int i = 0; i < depth ; i++)
    {
        scale /= 2.0f;
        boundaries.RenderMask(mXs[i].Front, scale);
        boundaries.RenderMask(mXs[i].Back, scale);
        boundaries.RenderWeights(mWeightss[i], mQuads[i]);
    }
}

void Multigrid::Render(Renderer::Program & program)
{
    mXs[0].begin();
    program.Use().SetMVP(mXs[0].Orth);
    mQuads[0].Render();
    program.Unuse();
    mXs[0].end();
}

void Multigrid::BindWeights(int n)
{
    mWeightss[0].Bind(n);
}

void Multigrid::BindPressure(int n)
{
    mXs[0].Front.Bind(n);
}

void Multigrid::Solve()
{
    for(int i = 0 ; i < depth - 1 ; i++)
    {
        DampedJacobi(i);
        Residual(i);
        Restrict(i);
    }
    DampedJacobi(depth - 1);
    for(int i = depth - 2 ; i >= 0 ; --i)
    {
        Prolongate(i);
        Correct(i);
        DampedJacobi(i);
    }
}

void Multigrid::DampedJacobi(int depth)
{
    auto & x = mXs[depth];
    auto & weights = mWeightss[depth];
    auto & quad = mQuads[depth];

    Renderer::Enable e(GL_STENCIL_TEST);
    glStencilMask(0x00);
    glStencilFunc(GL_EQUAL, 0, 0xFF);

    for(int i = 0 ; i < jacobi ; i++)
    {
        x.swap();
        x.begin();

        mDampedJacobiShader.Use().Set("h", quad.Size()).SetMVP(x.Orth);

        weights.Bind(1);
        x.Back.Bind(0);

        quad.Render();

        mDampedJacobiShader.Unuse();
        
        x.end();
    }

}

void Multigrid::Residual(int depth)
{
    auto & x = mXs[depth];
    auto & weights = mWeightss[depth];
    auto & quad = mQuads[depth];

    Renderer::Enable e(GL_STENCIL_TEST);
    glStencilMask(0x00);
    glStencilFunc(GL_EQUAL, 0, 0xFF);

    x.swap();
    x.begin();

    mResidualShader.Use().Set("h", quad.Size()).SetMVP(x.Orth);

    weights.Bind(1);
    x.Back.Bind(0);

    quad.Render();

    mResidualShader.Unuse();
    
    x.end();
}

void Multigrid::Restrict(int depth)
{
    auto & x = mXs[depth];
    auto & r = mXs[depth+1];
    auto & quad = mQuads[depth+1];

    r.begin();

    mRestrictShader.Use().Set("h", quad.Size()).SetMVP(r.Orth);

    x.Front.Bind(0);

    quad.Render();

    mRestrictShader.Unuse();

    r.end();
}

void Multigrid::Prolongate(int depth)
{
    auto & x = mXs[depth];
    auto & u = mXs[depth+1];
    auto & quad = mQuads[depth];

    x.begin();

    mProlongateShader.Use().Set("h", quad.Size()).SetMVP(x.Orth);

    x.Back.Bind(1);
    u.Front.Bind(0);

    quad.Render();

    mProlongateShader.Unuse();

    x.end();
}

void Multigrid::Correct(int depth)
{
    auto & x = mXs[depth];
    auto & quad = mQuads[depth];

    x.swap();
    x.begin();

    mCorrectShader.Use().Set("h", quad.Size()).SetMVP(x.Orth);

    x.Back.Bind(0);

    quad.Render();

    mCorrectShader.Unuse();

    x.end();
}

}