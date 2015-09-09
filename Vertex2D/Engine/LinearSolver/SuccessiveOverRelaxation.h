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
                             Boundaries & boundaries,
                             int iterations = 20);


    void RenderMask(const std::vector<Renderer::Drawable*> & objects);
    void Solve() override;

private:
    void Step(bool isRed);

    Renderer::Quad mQuad;
    Renderer::RenderTexture & mWeights;
    Renderer::PingPong & mX;
    Boundaries & mBoundaries;
    int mIterations;

    Renderer::Program mSorShader;
    Renderer::Program mStencilShader;
    Renderer::Program & mIdentityShader;
};

}

#endif /* defined(__Vertex2D__SuccessiveOverRelaxation__) */
