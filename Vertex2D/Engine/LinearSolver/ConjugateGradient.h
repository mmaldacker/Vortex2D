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

namespace Fluid
{

class ConjugateGradient : public LinearSolver
{
public:
    void Init(Boundaries & boundaries) override;
    LinearSolver::Data & GetData() override;
    void Solve() override;

private:
    Multigrid mMultigrid;
};

}

#endif /* defined(__Vertex2D__ConjugateGradient__) */
