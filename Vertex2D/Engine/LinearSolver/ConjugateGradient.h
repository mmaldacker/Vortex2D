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
#include "Multigrid.h"
#include "Reduce.h"

namespace Fluid
{

class ConjugateGradient : public LinearSolver
{
public:
    ConjugateGradient(const glm::vec2 & size);

    void Init(LinearSolver::Data & data, Boundaries & boundaries) override;
    void Solve(LinearSolver::Data & data) override;

    void NormalSolve(LinearSolver::Data & data);

private:
    Buffer r, s, alpha, beta, rho, rho_new, sigma;
    Operator matrixMultiply, scalarDivision, multiplyAdd, multiplySub, residual, identity, swizzle;
    Reduce reduce;
    Multigrid preconditioner;
    LinearSolver::Data z;
};

}

#endif /* defined(__Vertex2D__ConjugateGradient__) */