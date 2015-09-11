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
#include "Size.h"
#include "RenderTexture.h"
#include "Boundaries.h"

namespace Fluid
{

class SuccessiveOverRelaxation : public LinearSolver
{
public:
    SuccessiveOverRelaxation(Dimensions dimensions,
                             Renderer::RenderTexture & weights,
                             Renderer::PingPong & x,
                             int iterations = 40);


    void RenderMask(Boundaries & boundaries);
    void Solve() override;

private:
    void Step(bool isRed);

    Renderer::Quad mQuad;
    Renderer::RenderTexture & mWeights;
    Renderer::PingPong & mX;
    int mIterations;

    Renderer::Program mSorShader;
    Renderer::Program mStencilShader;
    Renderer::Program & mIdentityShader;
};

}

#endif /* defined(__Vertex2D__SuccessiveOverRelaxation__) */
