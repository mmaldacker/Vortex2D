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
    SuccessiveOverRelaxation(const glm::vec2 & size, int iterations = 40);
    SuccessiveOverRelaxation(const glm::vec2 & size, int iterations, float w);

    void Init(Boundaries & boundaries) override;
    LinearSolver::Data & GetData() override;
    void Solve() override;

//private:
    void Step(bool isRed);

    LinearSolver::Data mData;
    int mIterations;

    Renderer::Program mSorShader;
    Renderer::Program mStencilShader;
    Renderer::Program & mIdentityShader;
};

}

#endif /* defined(__Vertex2D__SuccessiveOverRelaxation__) */
