//
//  LinearSolver.h
//  Vertex2D
//
//  Created by Maximilian Maldacker on 19/08/2015.
//  Copyright (c) 2015 Maximilian Maldacker. All rights reserved.
//

#ifndef Vertex2D_LinearSolver_h
#define Vertex2D_LinearSolver_h

#include "Boundaries.h"

namespace Fluid
{

class LinearSolver
{
public:
    virtual void Init(Boundaries & boundaries) = 0;
    virtual void Solve() = 0;
};

}

#endif
