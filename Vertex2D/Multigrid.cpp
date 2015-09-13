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
int jacobi = 20;

Multigrid::Multigrid(Dimensions dimensions,
                     Renderer::RenderTexture & weights,
                     Renderer::PingPong & x)
    : mX(x)
    , mWeights(weights)
    , mQuad(dimensions.Size)
    , mCorrectShader("Diff.vsh", "Correct.fsh")
    , mDampedJacobiShader("Diff.vsh", "DampedJacobi.fsh")
    , mProlongateShader("Diff.vsh", "Prolongate.fsh")
    , mResidualShader("Diff.vsh", "Residual.fsh")
    , mRestrictShader("Restrict.vsh", "Restrict.fsh")
{
    auto size = dimensions.Size;
    
    for(int i = 1 ; i < depth ; ++i)
    {
        size /= glm::vec2(2.0f);
        mQuads.emplace_back(size);
        mXs.emplace_back(size.x, size.y, Renderer::Texture::PixelFormat::RGF, Renderer::RenderTexture::DepthFormat::DEPTH24_STENCIL8);
        mXs.back().Clear();
        mXs.back().Front.SetAntiAliasTexParameters();
        mXs.back().Back.SetAntiAliasTexParameters();
        mWeightss.emplace_back(size.x, size.y, Renderer::Texture::PixelFormat::RGBAF);
        mWeightss.back().Clear();
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

template<typename T>
T & Multigrid::Get(int depth, T & s, std::vector<T> & l)
{
    if(depth == 0)
    {
        return s;
    }
    else
    {
        return l[depth-1];
    }
}

Renderer::PingPong & Multigrid::GetX(int depth)
{
    return Get(depth, mX, mXs);
}

Renderer::RenderTexture & Multigrid::GetWeights(int depth)
{
    return Get(depth, mWeights, mWeightss);
}

Renderer::Quad & Multigrid::GetQuad(int depth)
{
    return Get(depth, mQuad, mQuads);
}

Renderer::Reader Multigrid::GetPressureReader(int depth)
{
    return {Get(depth, mX, mXs).Front};
}

void Multigrid::Init(Boundaries & boundaries)
{
    boundaries.RenderMask(mX.Front);
    boundaries.RenderMask(mX.Back);

    float scale = 1.0f;
    for(int i = 0; i < depth -1 ; i++)
    {
        scale /= 2.0f;
        boundaries.RenderMask(mXs[i].Front, scale);
        boundaries.RenderMask(mXs[i].Back, scale);
        boundaries.RenderWeights(mWeightss[i], mQuads[i]);
    }
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
    auto & x = GetX(depth);
    auto & weights = GetWeights(depth);
    auto & quad = GetQuad(depth);

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
    auto & x = GetX(depth);
    auto & weights = GetWeights(depth);
    auto & quad = GetQuad(depth);

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
    auto & x = GetX(depth);
    auto & r = GetX(depth+1);

    r.begin();

    mRestrictShader.Use().Set("h", GetQuad(depth+1).Size()).SetMVP(r.Orth);

    x.Front.Bind(0);

    GetQuad(depth+1).Render();

    mRestrictShader.Unuse();

    r.end();
}

void Multigrid::Prolongate(int depth)
{
    auto & x = GetX(depth);
    auto & u = GetX(depth+1);
    auto & quad = GetQuad(depth);

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
    auto & x = GetX(depth);
    auto & quad = GetQuad(depth);

    x.swap();
    x.begin();

    mCorrectShader.Use().Set("h", quad.Size()).SetMVP(x.Orth);

    x.Back.Bind(0);

    quad.Render();

    mCorrectShader.Unuse();

    x.end();
}

}