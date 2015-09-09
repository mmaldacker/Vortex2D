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

SuccessiveOverRelaxation::SuccessiveOverRelaxation(Dimensions dimensions,
                                                   Renderer::RenderTexture & weights,
                                                   Renderer::PingPong & x,
                                                   Boundaries & boundaries,
                                                   int iterations)
    : mQuad(dimensions.Size)
    , mWeights(weights)
    , mX(x)
    , mBoundaries(boundaries)
    , mIterations(iterations)
    , mSorShader("Diff.vsh", "SOR.fsh")
    , mStencilShader("Diff.vsh", "Stencil.fsh")
    , mIdentityShader(Renderer::Program::TexturePositionProgram())
{
    float w = 2.0f/(1.0f+std::sin(4.0f*std::atan(1.0f)/std::sqrt(mQuad.Size().x*mQuad.Size().y)));

    mSorShader.Use()
    .Set("h", mQuad.Size())
    .Set("u_texture", 0)
    .Set("u_weights", 1)
    .Set("w", w)
    .Unuse();

    mStencilShader.Use()
    .Set("h", mQuad.Size())
    .Unuse();
}

void SuccessiveOverRelaxation::RenderMask(const std::vector<Renderer::Drawable*> & objects)
{
    mBoundaries.RenderMask(mX.Front, objects);
    mBoundaries.RenderMask(mX.Back, objects);

    Renderer::Enable e(GL_STENCIL_TEST);

    glStencilFunc(GL_NOTEQUAL, 1, 0xFF); // write value in stencil buffer
    glStencilOp(GL_KEEP, GL_KEEP, GL_INVERT); // invert value
    glStencilMask(0x02); // write in second place
    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

    mStencilShader.Use().SetMVP(mX.Orth);

    mX.begin();

    mQuad.Render();

    mX.end();
    mX.swap();
    mX.begin();

    mQuad.Render();

    mX.end();
    mX.swap();

    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    glStencilMask(0x00); // disable stencil writing

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

    mX.swap();
    mX.begin({0.0f, 0.0f, 0.0f, 0.0f});

    glStencilFunc(GL_EQUAL, isRed ? 2 : 0, 0xFF);

    mSorShader.Use().SetMVP(mX.Orth);

    mWeights.Bind(1);
    mX.Back.Bind(0);

    mQuad.Render();

    mSorShader.Unuse();

    glStencilFunc(GL_EQUAL, isRed ? 0 : 2, 0xFF);

    mIdentityShader.Use().SetMVP(mX.Orth);
    
    mQuad.Render();

    mIdentityShader.Unuse();

    mX.end();
}

}