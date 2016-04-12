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
    : mIterations(iterations)
    , mSor("TexturePosition.vsh", "SOR.fsh")
    , mStencil("TexturePosition.vsh", "Stencil.fsh")
    , mIdentity("TexturePosition.vsh", "TexturePosition.fsh")
{
    float w = 2.0f/(1.0f+std::sin(4.0f*std::atan(1.0f)/std::sqrt(size.x*size.y)));

    mSor.Use().Set("u_texture", 0).Set("u_weights", 1).Set("u_diagonals", 2).Set("w", w).Unuse();
}

SuccessiveOverRelaxation::SuccessiveOverRelaxation(const glm::vec2 & size, int iterations, float w)
    : SuccessiveOverRelaxation(size, iterations)
{
    mSor.Use().Set("w", w).Unuse();
}

void SuccessiveOverRelaxation::Init(LinearSolver::Data & data, Boundaries & boundaries)
{
    data.Weights = boundaries.GetWeights();
    data.Diagonal = boundaries.GetDiagonals();

    Renderer::Enable e(GL_STENCIL_TEST);
    Renderer::DisableColorMask c;

    glStencilFunc(GL_NOTEQUAL, 1, 0xFF); // write value in stencil buffer
    glStencilOp(GL_KEEP, GL_KEEP, GL_INVERT); // invert value
    glStencilMask(0x02); // write in second place

    data.Pressure = mStencil();
    data.Pressure.swap();
    data.Pressure = mStencil();

    glStencilMask(0x00); // disable stencil writing
}

void SuccessiveOverRelaxation::Solve(LinearSolver::Data & data)
{
    Solve(data, true);
}

void SuccessiveOverRelaxation::Solve(LinearSolver::Data & data, bool up)
{
    for (int i  = 0; i < mIterations; ++i)
    {
        Step(data, up);
        Step(data, !up);
    }
}

void SuccessiveOverRelaxation::Step(LinearSolver::Data & data, bool isRed)
{
    Renderer::Enable e(GL_STENCIL_TEST);
    glStencilMask(0x00);

    data.Pressure.swap();

    glStencilFunc(GL_EQUAL, isRed ? 2 : 0, 0xFF);
    data.Pressure = mSor(Back(data.Pressure), data.Weights, data.Diagonal);

    glStencilFunc(GL_EQUAL, isRed ? 0 : 2, 0xFF);
    data.Pressure = mIdentity(Back(data.Pressure), data.Weights, data.Diagonal);
}

}