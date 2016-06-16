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

namespace Fluid
{

/**
 * @brief An iterative black and red successive over relaxation linear solver.
 */
class SuccessiveOverRelaxation : public LinearSolver
{
public:
    SuccessiveOverRelaxation(const glm::vec2 & size, int iterations = 40);
    SuccessiveOverRelaxation(const glm::vec2 & size, int iterations, float w);

    void Init(LinearSolver::Data & data) override;
    void Solve(LinearSolver::Data & data) override;

private:
    void Step(LinearSolver::Data & data, bool isRed);

    int mIterations;

    Operator mSor;
    Operator mStencil;
    Operator mIdentity;
};

}

#endif /* defined(__Vertex2D__SuccessiveOverRelaxation__) */
