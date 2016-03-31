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
    , swizzle("TexturePosition.vsh", "Swizzle.fsh")
    , reduce(size)
    , z(size)
{
    residual.Use().Set("u_texture", 0).Set("u_weights", 1).Set("u_diagonals", 2).Unuse();
    identity.Use().Set("u_texture", 0).Unuse();
    matrixMultiply.Use().Set("u_texture", 0).Set("u_weights", 1).Set("u_diagonals", 2).Unuse();
    scalarDivision.Use().Set("u_texture", 0).Set("u_other", 1).Unuse();
    multiplyAdd.Use().Set("u_texture", 0).Set("u_other", 1).Set("u_scalar", 2).Unuse();
    multiplySub.Use().Set("u_texture", 0).Set("u_other", 1).Set("u_scalar", 2).Unuse();
}

void ConjugateGradient::Init(LinearSolver::Data & data, Boundaries & boundaries)
{
    data.Pressure.clear();
    z.Pressure.clear();
    r.clear();
    s.clear();
    alpha.clear();
    beta.clear();
    rho.clear();
    rho_new.clear();
    sigma.clear();

    boundaries.RenderMask(data.Pressure);
    data.Pressure.swap();
    boundaries.RenderMask(data.Pressure);
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

    // z is result from preconditioner from r

    // s = z
    s = identity(z.Pressure);

    // rho = zTr
    rho = reduce(z.Pressure,r);

    for(int i = 0 ; i < 5; ++i)
    {
        // z = Ap
        z.Pressure = matrixMultiply(s, data.Weights, data.Diagonal);

        // alpha = rho / zTs
        sigma = reduce(z.Pressure,s);
        alpha = scalarDivision(rho, sigma);

        // p = p + alpha * s
        data.Pressure.swap();
        data.Pressure = multiplyAdd(Back(data.Pressure), s, alpha);

        // r = r - alpha * z
        r.swap();
        r = multiplySub(Back(r), z.Pressure, alpha);

        // z is result from preconditioner from r

        // rho_new = zTr
        rho_new = reduce(z.Pressure,r);

        // s = z + beta * s
        s.swap();
        s = multiplyAdd(z.Pressure,Back(s),beta);

        // rho = rho_new
        rho = identity(rho_new);
    }
}

}