//
//  ConjugateGradient.cpp
//  Vertex2D
//
//  Created by Maximilian Maldacker on 15/09/2015.
//  Copyright (c) 2015 Maximilian Maldacker. All rights reserved.
//

#include "ConjugateGradient.h"

namespace Fluid
{

ConjugateGradient::ConjugateGradient(const glm::vec2 & size)
    : mData(size)
    , r(size.x, size.y, Renderer::Texture::PixelFormat::RGF, Renderer::RenderTexture::DepthFormat::DEPTH24_STENCIL8)
    , p(size.x, size.y, Renderer::Texture::PixelFormat::RGF, Renderer::RenderTexture::DepthFormat::DEPTH24_STENCIL8)
    , z(size.x, size.y, Renderer::Texture::PixelFormat::RGF, Renderer::RenderTexture::DepthFormat::DEPTH24_STENCIL8)
    , alpha(1,1,Renderer::Texture::PixelFormat::RF)
    , beta(1,1,Renderer::Texture::PixelFormat::RF)
    , matrixMultiply("Diff.vsh", "MultiplyMatrix.fsh")
    , scalarDivision("TexturePosition.vsh", "Divide.fsh")
    , multiplyAdd("TexturePosition.vsh", "MultiplyAdd.fsh")
    , multiplySub("TexturePosition.vsh", "MultiplySub.fsh")
    , residual("Diff.vsh", "Residual.fsh")
    , identity("TexturePosition.vsh", "TexturePosition.fsh")
    , pReduce(size)
    , rReduce(size)
{
    residual.Use().Set("u_texture", 0).Set("u_weights", 1).Unuse();
    identity.Use().Set("u_texture", 0).Unuse();
    matrixMultiply.Use().Set("u_texture", 0).Set("u_weights", 1).Unuse();
    scalarDivision.Use().Set("u_texture", 0).Set("u_other", 1).Unuse();
    multiplyAdd.Use().Set("u_texture", 0).Set("u_other", 1).Set("u_alpha", 2).Unuse();
    multiplySub.Use().Set("u_texture", 0).Set("u_other", 1).Set("u_alpha", 2).Unuse();
}

void ConjugateGradient::Init(Boundaries & boundaries)
{

}

LinearSolver::Data & ConjugateGradient::GetData()
{
    return mData;
}

void ConjugateGradient::Solve()
{
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

void ConjugateGradient::NormalSolve()
{
    // r = b - Ax
    residual.apply(mData.Quad, r, mData.Pressure.Front, mData.Weights);

    // p = r
    identity.apply(mData.Quad, p, r.Front);

    // rho = pTr
    rReduce.apply(p.Front, r.Front);
 
    for(int i = 0 ; i < 10; ++i)
    {
        // z = Ap
        matrixMultiply.apply(mData.Quad, z, p.Front, mData.Weights);

        // alpha = rho / pTz
        pReduce.apply(z, p.Front);
        scalarDivision.apply(mData.Quad, alpha, rReduce, pReduce);

        // r = r - alpha z
        r.swap();
        multiplySub.apply(mData.Quad, r, r.Back, z, alpha);

        // rho_new = rTr
        pReduce.apply(r.Front, r.Front);

        // beta = rho_new / rho
        scalarDivision.apply(mData.Quad, beta, rReduce, pReduce);

        // rho = rho_new
        std::swap(pReduce, rReduce);

        // x = x + alpha p
        mData.Pressure.swap();
        multiplyAdd.apply(mData.Quad, mData.Pressure, mData.Pressure.Back, p.Front, alpha);

        // p = r + beta p
        p.swap();
        multiplyAdd.apply(mData.Quad, p, r.Front, p.Back, beta);
    }
}


}