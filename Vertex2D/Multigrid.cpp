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

Multigrid::Multigrid(const glm::vec2 & s, int iterations)
    : mDepths(0)
    , mCorrectShader("Diff.vsh", "Correct.fsh")
    , mProlongateShader("Diff.vsh", "Prolongate.fsh")
    , mResidualShader("Diff.vsh", "Residual.fsh")
    , mRestrictShader("Diff.vsh", "Restrict.fsh")
{
    auto size = s;
    const float min_size = 4.0f;

    do
    {
        mXs.emplace_back(size, iterations, 1.0f);

        //size = ((size - glm::vec2(2.0)) / glm::vec2(2.0f)) + glm::vec2(2.0f);
        size /= glm::vec2(2.0f);
        mDepths++;
    }while(size.x > min_size && size.y > min_size);

    mCorrectShader.Use()
    .Set("u_texture", 0)
    .Set("u_residual", 1)
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
    return {mXs[depth].mX.Front};
}

void Multigrid::Init(Boundaries & boundaries)
{
    for(int i = 0; i < mDepths ; i++)
    {
        mXs[i].Init(boundaries);
    }
}

void Multigrid::Render(Renderer::Program & program)
{
    mXs[0].Render(program);
}

void Multigrid::BindWeights(int n)
{
    mXs[0].BindWeights(n);
}

Renderer::PingPong & Multigrid::GetPressure()
{
    return mXs[0].GetPressure();
}

void Multigrid::Solve()
{
    for(int i = 0 ; i < mDepths - 1 ; i++)
    {
        DampedJacobi(i);
        Residual(i);
        Restrict(i);
    }
    DampedJacobi(mDepths - 1);
    for(int i = mDepths - 2 ; i >= 0 ; --i)
    {
        Prolongate(i);
        Correct(i);
        DampedJacobi(i);
    }
}

void Multigrid::DampedJacobi(int depth)
{
    mXs[depth].Solve();
}

void Multigrid::Residual(int depth)
{
    auto & x = mXs[depth];

    Renderer::Enable e(GL_STENCIL_TEST);
    glStencilMask(0x00);
    glStencilFunc(GL_NOTEQUAL, 1, 0xFF);

    x.GetPressure().swap();
    x.GetPressure().begin();
    x.BindWeights(1);
    x.GetPressure().Back.Bind(0);
    x.Render(mResidualShader);
    x.GetPressure().end();
}

void Multigrid::Restrict(int depth)
{
    auto & x = mXs[depth];
    auto & r = mXs[depth+1];

    Renderer::Enable e(GL_STENCIL_TEST);
    glStencilMask(0x00);
    glStencilFunc(GL_NOTEQUAL, 1, 0xFF);

    r.GetPressure().begin();
    x.GetPressure().Front.Bind(0);
    r.Render(mRestrictShader);
    r.GetPressure().end();
}

void Multigrid::Prolongate(int depth)
{
    auto & x = mXs[depth];
    auto & u = mXs[depth+1];

    Renderer::Enable e(GL_STENCIL_TEST);
    glStencilMask(0x00);
    glStencilFunc(GL_NOTEQUAL, 1, 0xFF);

    x.GetPressure().begin();
    x.GetPressure().Back.Bind(1);
    u.GetPressure().Front.Bind(0);
    x.Render(mProlongateShader);
    x.GetPressure().end();
}

void Multigrid::Correct(int depth)
{
    auto & x = mXs[depth];

    Renderer::Enable e(GL_STENCIL_TEST);
    glStencilMask(0x00);
    glStencilFunc(GL_NOTEQUAL, 1, 0xFF);

    x.GetPressure().swap();
    x.GetPressure().begin();
    x.GetPressure().Back.Bind(0);
    x.Render(mCorrectShader);
    x.GetPressure().end();
}

}