//
//  Multigrid.h
//  Vertex2D
//
//  Created by Maximilian Maldacker on 10/09/2015.
//  Copyright (c) 2015 Maximilian Maldacker. All rights reserved.
//

#ifndef __Vertex2D__Multigrid__
#define __Vertex2D__Multigrid__

#include "Size.h"
#include "RenderTexture.h"
#include "Boundaries.h"
#include "LinearSolver.h"
#include "SuccessiveOverRelaxation.h"
#include "Operator.h"

namespace Fluid
{

class Multigrid : public LinearSolver
{
public:
    Multigrid(const glm::vec2 & size, int iterations = 5);

    void Init(Boundaries & boundaries) override;
    LinearSolver::Data & GetData() override;
    void Solve() override;

    Renderer::Reader GetPressureReader(int depth);
    Renderer::Reader GetWeightsReader(int depth);

//private:
    void GaussSeidel(int depth, bool up);
    void Residual(int depth);
    void Restrict(int depth);
    void Prolongate(int depth);
    void Correct(int depth);

    int mDepths;

    std::vector<SuccessiveOverRelaxation> mXs;

    Operator mCorrect;
    Operator mProlongate;
    Operator mResidual;
    Operator mRestrict;
};

}

#endif /* defined(__Vertex2D__Multigrid__) */
