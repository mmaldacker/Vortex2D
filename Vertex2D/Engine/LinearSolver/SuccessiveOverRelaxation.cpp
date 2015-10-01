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
    , mSor("TexturePosition.vsh", "SOR.fsh")
    , mStencil("TexturePosition.vsh", "Stencil.fsh")
    , mIdentity("TexturePosition.vsh", "TexturePosition.fsh")
{
    float w = 2.0f/(1.0f+std::sin(4.0f*std::atan(1.0f)/std::sqrt(mData.Pressure.size().x*mData.Pressure.size().y)));

    mData.Weights.clear();
    mData.Pressure.clear();

    mSor.Use().Set("u_texture", 0).Set("u_weights", 1).Set("w", w).Unuse();
}

SuccessiveOverRelaxation::SuccessiveOverRelaxation(const glm::vec2 & size, int iterations, float w)
    : SuccessiveOverRelaxation(size, iterations)
{
    mSor.Use().Set("w", w).Unuse();
}

void SuccessiveOverRelaxation::Init(Boundaries & boundaries)
{
    boundaries.RenderMask(mData.Pressure);
    mData.Pressure.swap();
    boundaries.RenderMask(mData.Pressure);
    mData.Weights = boundaries.GetWeights();

    Renderer::Enable e(GL_STENCIL_TEST);
    Renderer::DisableColorMask c;

    glStencilFunc(GL_NOTEQUAL, 1, 0xFF); // write value in stencil buffer
    glStencilOp(GL_KEEP, GL_KEEP, GL_INVERT); // invert value
    glStencilMask(0x02); // write in second place

    mData.Pressure = mStencil();
    mData.Pressure.swap();
    mData.Pressure = mStencil();

    glStencilMask(0x00); // disable stencil writing
}

LinearSolver::Data & SuccessiveOverRelaxation::GetData()
{
    return mData;
}

void SuccessiveOverRelaxation::Solve()
{
    Solve(true);
}

void SuccessiveOverRelaxation::Solve(bool up)
{
    for (int i  = 0; i < mIterations; ++i)
    {
        Step(up);
        Step(!up);
    }
}

void SuccessiveOverRelaxation::Step(bool isRed)
{
    Renderer::Enable e(GL_STENCIL_TEST);
    glStencilMask(0x00);

    mData.Pressure.swap();

    glStencilFunc(GL_EQUAL, isRed ? 2 : 0, 0xFF);
    mData.Pressure = mSor(Back(mData.Pressure), mData.Weights);

    glStencilFunc(GL_EQUAL, isRed ? 0 : 2, 0xFF);
    mData.Pressure = mIdentity(Back(mData.Pressure), mData.Weights);
}

}