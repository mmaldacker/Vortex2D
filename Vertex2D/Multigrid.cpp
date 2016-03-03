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
    , mCorrect("TexturePosition.vsh", "Correct.fsh")
    , mProlongate("TexturePosition.vsh", "Prolongate.fsh")
    , mResidual("Diff.vsh", "Residual.fsh")
    , mRestrict("TexturePosition.vsh", "Restrict.fsh")
{
    auto size = s;
    const float min_size = 4.0f;

    do
    {
        mXs.emplace_back(size, iterations, 0.66f);
        mDatas.emplace_back(size);
        mDatas.back().Pressure.linear();
        size = glm::ceil(glm::vec2(1.0)+size/glm::vec2(2.0f));
        mDepths++;
    }while(size.x > min_size && size.y > min_size);

    mCorrect.Use().Set("u_texture", 0).Set("u_residual", 1).Unuse();
    mProlongate.Use().Set("u_texture", 0).Set("u_pressure", 1).Unuse();
    mResidual.Use().Set("u_texture", 0).Set("u_weights", 1).Unuse();
    mRestrict.Use().Set("u_texture", 0).Unuse();
}

LinearSolver::Data & Multigrid::GetData(int depth)
{
    if(depth == 0)
    {
        return *mDataPtr;
    }
    else
    {
        return mDatas[depth];
    }
}

void Multigrid::Init(LinearSolver::Data & data, Boundaries & boundaries)
{
    data.Pressure.linear();
    mDataPtr = &data;

    for(int i = 0; i < mDepths ; i++)
    {
        mXs[i].Init(GetData(i), boundaries);
    }
}

void Multigrid::Solve(LinearSolver::Data & data)
{
    mDataPtr = &data;

    Renderer::Enable e(GL_STENCIL_TEST);
    glStencilMask(0x00);
    glStencilFunc(GL_NOTEQUAL, 1, 0xFF);

    for(int i = 0 ; i < mDepths - 1 ; i++)
    {
        auto & x = GetData(i);

        mXs[i].Solve(x, true);
        // FIXME Solve should restore stencilFunc
        glStencilFunc(GL_NOTEQUAL, 1, 0xFF);

        x.Pressure.swap();
        x.Pressure = mResidual(Back(x.Pressure), x.Weights);

        auto & r = GetData(i+1);
        r.Pressure = mRestrict(x.Pressure);
    }

    mXs[mDepths - 1].Solve(GetData(mDepths - 1), true);
    // FIXME Solve should restore stencilFunc
    glStencilFunc(GL_NOTEQUAL, 1, 0xFF);

    for(int i = mDepths - 2 ; i >= 0 ; --i)
    {
        auto & x = GetData(i);
        auto & u = GetData(i+1);
        x.Pressure = mProlongate(u.Pressure, Back(x.Pressure));

        x.Pressure.swap();
        x.Pressure = mCorrect(Back(x.Pressure));

        mXs[i].Solve(x, false);
        // FIXME Solve should restore stencilFunc
        glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
    }
}

}