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
#include "Reduce.h"

namespace Fluid
{

/**
 * @brief An iterative preconditioned conjugate linear solver.
 * The preconditioning operator is a simple diagonal preconditioner.
 */
class ConjugateGradient : public LinearSolver
{
public:
    ConjugateGradient(const glm::vec2 & size);

    /**
     * @brief Empty implementation as there are no initialisation for CG
     */
    void Init(LinearSolver::Data & data) override;

    /**
     * @brief Solve iteratively solve the linear equations in data
     */
    void Solve(LinearSolver::Data & data) override;

private:
    Buffer r, s, z, alpha, beta, rho, rho_new, sigma;
    Operator matrixMultiply, scalarDivision, multiplyAdd, multiplySub, residual, identity;
    Reduce reduce;
};

}

#endif /* defined(__Vertex2D__ConjugateGradient__) */
