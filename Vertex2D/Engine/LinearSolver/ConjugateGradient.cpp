//
//  ConjugateGradient.cpp
//  Vertex2D
//
//  Created by Maximilian Maldacker on 15/09/2015.
//  Copyright (c) 2015 Maximilian Maldacker. All rights reserved.
//

#include "ConjugateGradient.h"
#include "Disable.h"

namespace Fluid
{

ConjugateGradient::ConjugateGradient(const glm::vec2 & size)
    : r(size, 1, true)
    , p(size, 1, true)
    , z(size, 1)
    , alpha({1,1}, 1)
    , beta({1,1}, 1)
    , rho({1,1}, 1)
    , rho_new({1,1}, 1)
    , sigma({1,1}, 1)
    , matrixMultiply("TexturePosition.vsh", "MultiplyMatrix.fsh")
    , scalarDivision("TexturePosition.vsh", "Divide.fsh")
    , multiplyAdd("TexturePosition.vsh", "MultiplyAdd.fsh")
    , multiplySub("TexturePosition.vsh", "MultiplySub.fsh")
    , residual("Diff.vsh", "Residual.fsh")
    , identity("TexturePosition.vsh", "TexturePosition.fsh")
    , reduce(size)
{
    residual.Use().Set("u_texture", 0).Set("u_weights", 1).Unuse();
    identity.Use().Set("u_texture", 0).Unuse();
    matrixMultiply.Use().Set("u_texture", 0).Set("u_weights", 1).Unuse();
    scalarDivision.Use().Set("u_texture", 0).Set("u_other", 1).Unuse();
    multiplyAdd.Use().Set("u_texture", 0).Set("u_other", 1).Set("u_scalar", 2).Unuse();
    multiplySub.Use().Set("u_texture", 0).Set("u_other", 1).Set("u_scalar", 2).Unuse();
}

void ConjugateGradient::Init(LinearSolver::Data & data, Boundaries & boundaries)
{
    data.Pressure.clear();
    r.clear();
    p.clear();
    // FIXME needed above?

    boundaries.RenderMask(data.Pressure);
    data.Pressure.swap();
    boundaries.RenderMask(data.Pressure);
    data.Weights = boundaries.GetWeights();
}

void ConjugateGradient::Solve(LinearSolver::Data & data)
{
    NormalSolve(data);
    
    // initialise
    // x is solution

    // r = b - Ax
    // p is result from multigrid from r
    // rho = pTr

    // iterate

    // z = Ap
    // alpha = rho / pTz
    // r = r - alpha z
    // z is result from multigrid from r
    // rho_new = zTr
    // beta = rho_new / rho
    // rho = rho_new
    // x = x + alpha p
    // p = z + beta p

}

void ConjugateGradient::NormalSolve(LinearSolver::Data & data)
{
    Renderer::Enable e(GL_STENCIL_TEST);
    glStencilMask(0x00);
    glStencilFunc(GL_NOTEQUAL, 1, 0xFF);

    // r = b - Ax
    r = residual(data.Pressure, data.Weights);
    
    // p = r
    p = identity(r);

    // rho = pTr
    rho = reduce(p,r);

    for(int i = 0 ; i < 40; ++i)
    {
        // z = Ap
        z = matrixMultiply(p, data.Weights);

        // sigma = pTz
        sigma = reduce(p,z);

        // alpha = rho / sigma
        alpha = scalarDivision(rho, sigma);

        // x = x + alpha * p
        data.Pressure.swap();
        data.Pressure = multiplyAdd(Back(data.Pressure), p, alpha);

        // r = r - alpha * z
        r.swap();
        r = multiplySub(Back(r), z, alpha);

        // rho_new = rTr
        rho_new = reduce(r,r);

        // beta = rho_new / rho
        beta = scalarDivision(rho_new, rho);

        // p = r + beta * p
        p.swap();
        p = multiplyAdd(r, Back(p), beta);

        // rho = rho_new
        rho = identity(rho_new);
    }
}


}