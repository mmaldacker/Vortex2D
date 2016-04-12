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
    , s(size, 1, true)
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
    , residual("TexturePosition.vsh", "Residual.fsh")
    , identity("TexturePosition.vsh", "TexturePosition.fsh")
    , preconditioner("TexturePosition.vsh", "JacobiPreconditioner.fsh")
    , reduce(size)
{
    residual.Use().Set("u_texture", 0).Set("u_weights", 1).Set("u_diagonals", 2).Unuse();
    identity.Use().Set("u_texture", 0).Unuse();
    matrixMultiply.Use().Set("u_texture", 0).Set("u_weights", 1).Set("u_diagonals", 2).Unuse();
    scalarDivision.Use().Set("u_texture", 0).Set("u_other", 1).Unuse();
    multiplyAdd.Use().Set("u_texture", 0).Set("u_other", 1).Set("u_scalar", 2).Unuse();
    multiplySub.Use().Set("u_texture", 0).Set("u_other", 1).Set("u_scalar", 2).Unuse();
    preconditioner.Use().Set("u_texture", 0).Set("u_diagonals", 1).Unuse();
}

void ConjugateGradient::Init(LinearSolver::Data & data, Boundaries & boundaries)
{
    z.clear();
    r.clear();
    s.clear();
    alpha.clear();
    beta.clear();
    rho.clear();
    rho_new.clear();
    sigma.clear();

    data.Weights = boundaries.GetWeights();
    data.Diagonal = boundaries.GetDiagonals();
}

void ConjugateGradient::Solve(LinearSolver::Data & data)
{
    Renderer::Enable e(GL_STENCIL_TEST);
    glStencilMask(0x00);
    glStencilFunc(GL_NOTEQUAL, 1, 0xFF);

    // r = b - Ax
    r = residual(data.Pressure, data.Weights, data.Diagonal);

    // p = 0
    data.Pressure.clear();

    // z = M^-1 r
    z = preconditioner(r, data.Diagonal);

    // s = z
    s = identity(z);

    // rho = zTr
    rho = reduce(z,r);

    for(int i = 0 ; i < 40; ++i)
    {
        // z = Ap
        z = matrixMultiply(s, data.Weights, data.Diagonal);

        // alpha = rho / zTs
        sigma = reduce(z,s);
        alpha = scalarDivision(rho, sigma);

        // p = p + alpha * s
        data.Pressure.swap();
        data.Pressure = multiplyAdd(Back(data.Pressure), s, alpha);

        // r = r - alpha * z
        r.swap();
        r = multiplySub(Back(r), z, alpha);

        // z = M^-1 r
        z = preconditioner(r, data.Diagonal);

        // rho_new = zTr
        rho_new = reduce(z,r);

        // beta = rho_new / rho
        beta = scalarDivision(rho_new, rho);

        // s = z + beta * s
        s.swap();
        s = multiplyAdd(z,Back(s),beta);

        // rho = rho_new
        rho = identity(rho_new);
    }
}

}