//
//  ConjugateGradient.h
//  Vertex2D
//
//  Created by Maximilian Maldacker on 15/09/2015.
//  Copyright (c) 2015 Maximilian Maldacker. All rights reserved.
//

#ifndef __Vertex2D__ConjugateGradient__
#define __Vertex2D__ConjugateGradient__

#include "LinearSolver.h"
#include "Operator.h"
#include "Reduce.h"

namespace Fluid
{

class ConjugateGradient : public LinearSolver
{
public:
    ConjugateGradient(const glm::vec2 & size);

    void Init(Boundaries & boundaries) override;
    LinearSolver::Data & GetData() override;
    void Solve() override;

    void NormalSolve();

private:
    LinearSolver::Data mData;
    Renderer::PingPong r, p;
    Renderer::RenderTexture z, alpha, beta;
    Operator matrixMultiply, scalarDivision, multiplyAdd, multiplySub, residual, identity;
    Reduce pReduce, rReduce;
};

}

#endif /* defined(__Vertex2D__ConjugateGradient__) */
