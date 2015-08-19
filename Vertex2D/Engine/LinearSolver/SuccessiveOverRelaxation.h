//
//  SuccessiveOverRelaxation.h
//  Vertex2D
//
//  Created by Maximilian Maldacker on 19/08/2015.
//  Copyright (c) 2015 Maximilian Maldacker. All rights reserved.
//

#ifndef __Vertex2D__SuccessiveOverRelaxation__
#define __Vertex2D__SuccessiveOverRelaxation__

#include "LinearSolver.h"
#include "Quad.h"
#include "RenderTexture.h"

namespace Fluid
{

class SuccessiveOverRelaxation : public LinearSolver
{
public:
    SuccessiveOverRelaxation(Renderer::Quad & quad,
                             Renderer::PingPong & x,
                             Renderer::RenderTexture & weights,
                             int iterations = 20);

    void Init();
    void Solve() override;

private:
    void Step(bool isRed);

    Renderer::Quad & mQuad;
    Renderer::PingPong & mX;
    Renderer::RenderTexture & mWeights;
    int mIterations;

    Renderer::Program mSorShader;
    Renderer::Program mStencilShader;
    Renderer::Program & mIdentityShader;
};

}

#endif /* defined(__Vertex2D__SuccessiveOverRelaxation__) */
