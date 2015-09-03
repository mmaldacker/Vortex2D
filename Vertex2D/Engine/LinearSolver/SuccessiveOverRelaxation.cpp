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

SuccessiveOverRelaxation::SuccessiveOverRelaxation(Renderer::Quad & quad,
                                                   Renderer::PingPong & x,
                                                   Boundaries & boundaries,
                                                   int iterations)
    : mQuad(quad)
    , mX(x)
    , mBoundaries(boundaries)
    , mIterations(iterations)
    , mSorShader("Diff.vsh", "SOR.fsh")
    , mStencilShader("Diff.vsh", "Stencil.fsh")
    , mIdentityShader(Renderer::Program::TexturePositionProgram())
{
    float w = 2.0f/(1.0f+std::sin(4.0f*std::atan(1.0f)/std::sqrt(quad.Size().x*quad.Size().y)));

    mSorShader.Use()
    .Set("h", quad.Size())
    .Set("u_texture", 0)
    .Set("u_weights", 1)
    .Set("w", w)
    .Unuse();

    mStencilShader.Use()
    .Set("h", quad.Size())
    .Unuse();
}

void SuccessiveOverRelaxation::Init()
{
    glEnable(GL_STENCIL_TEST);
    glDisable(GL_DEPTH_TEST);
    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
    glDepthMask(GL_FALSE);
    glStencilMask(-1);
    glClearStencil(0);
    glStencilFunc(GL_ALWAYS, 1, -1);
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

    mX.begin({0.0f, 0.0f, 0.0f, 0.0f});
    mStencilShader.Use().SetMVP(mX.Orth);
    mQuad.Render();
    mStencilShader.Unuse();
    mX.end();

    mX.swap();

    mX.begin({0.0f, 0.0f, 0.0f, 0.0f});
    mQuad.Render();
    mX.end();

    glDisable(GL_STENCIL_TEST);
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    glStencilMask(0);
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
    GLenum equal = isRed ? GL_EQUAL : GL_NOTEQUAL;
    GLenum notEqual = isRed ? GL_NOTEQUAL : GL_EQUAL;

    Renderer::Enable e{GL_STENCIL_TEST};
    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

    mX.swap();
    mX.begin();

    glStencilFunc(equal, 1, -1);

    mSorShader.Use().SetMVP(mX.Orth);

    //FIXME
    //mWeights.Bind(1);
    mX.Back.Bind(0);

    mQuad.Render();

    mSorShader.Unuse();

    glStencilFunc(notEqual, 1, -1);

    mIdentityShader.Use().SetMVP(mX.Orth);
    
    mQuad.Render();

    mIdentityShader.Unuse();

    mX.end();
}

}