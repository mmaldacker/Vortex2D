//
//  SuccessiveOverRelaxation.cpp
//  Vertex2D
//
//  Created by Maximilian Maldacker on 19/08/2015.
//  Copyright (c) 2015 Maximilian Maldacker. All rights reserved.
//

#include "SuccessiveOverRelaxation.h"
#include "Common.h"
#include "Disable.h"

namespace Fluid
{

SuccessiveOverRelaxation::SuccessiveOverRelaxation(const glm::vec2 & size, int iterations)
    : mData(size)
    , mIterations(iterations)
    , mSorShader("Diff.vsh", "SOR.fsh")
    , mStencilShader("Diff.vsh", "Stencil.fsh")
    , mIdentityShader(Renderer::Program::TexturePositionProgram())
{
    float w = 2.0f/(1.0f+std::sin(4.0f*std::atan(1.0f)/std::sqrt(mData.Quad.Size().x*mData.Quad.Size().y)));

    mData.Weights.SetAliasTexParameters();

    mData.Weights.Clear();
    mData.Pressure.Clear();

    mSorShader.Use()
    .Set("h", mData.Quad.Size())
    .Set("u_texture", 0)
    .Set("u_weights", 1)
    .Set("w", w)
    .Unuse();

    mStencilShader.Use()
    .Set("h", mData.Quad.Size())
    .Unuse();
}

SuccessiveOverRelaxation::SuccessiveOverRelaxation(const glm::vec2 & size, int iterations, float w)
    : SuccessiveOverRelaxation(size, iterations)
{
    mSorShader.Use()
    .Set("w", w)
    .Unuse();
}

void SuccessiveOverRelaxation::Init(Boundaries & boundaries)
{
    boundaries.RenderMask(mData.Pressure.Front);
    boundaries.RenderMask(mData.Pressure.Back);
    boundaries.RenderWeights(mData.Weights, mData.Quad);

    Renderer::Enable e(GL_STENCIL_TEST);
    Renderer::DisableColorMask c;

    glStencilFunc(GL_NOTEQUAL, 1, 0xFF); // write value in stencil buffer
    glStencilOp(GL_KEEP, GL_KEEP, GL_INVERT); // invert value
    glStencilMask(0x02); // write in second place

    mStencilShader.Use().SetMVP(mData.Pressure.Orth);

    mData.Pressure.begin();

    mData.Quad.Render();

    mData.Pressure.end();
    mData.Pressure.swap();
    mData.Pressure.begin();

    mData.Quad.Render();

    mData.Pressure.end();
    mData.Pressure.swap();

    glStencilMask(0x00); // disable stencil writing
}

LinearSolver::Data & SuccessiveOverRelaxation::GetData()
{
    return mData;
}

void SuccessiveOverRelaxation::Solve()
{
    for (int i  = 0; i < mIterations; ++i)
    {
        Step(true);
        Step(false);
    }
}

void SuccessiveOverRelaxation::Step(bool isRed)
{
    Renderer::Enable e(GL_STENCIL_TEST);
    glStencilMask(0x00);

    mData.Pressure.swap();
    mData.Pressure.begin();

    glStencilFunc(GL_EQUAL, isRed ? 2 : 0, 0xFF);

    mSorShader.Use().SetMVP(mData.Pressure.Orth);

    mData.Weights.Bind(1);
    mData.Pressure.Back.Bind(0);

    mData.Quad.Render();

    mSorShader.Unuse();

    glStencilFunc(GL_EQUAL, isRed ? 0 : 2, 0xFF);

    mIdentityShader.Use().SetMVP(mData.Pressure.Orth);
    
    mData.Quad.Render();

    mIdentityShader.Unuse();

    mData.Pressure.end();
}

}