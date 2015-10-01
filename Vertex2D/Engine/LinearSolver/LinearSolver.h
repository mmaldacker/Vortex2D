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
#include "RenderTexture.h"
#include "Quad.h"

namespace Fluid
{

struct LinearSolver
{
    struct Data
    {
        Data(const glm::vec2 & size)
        : Weights(size, 4)
        , Pressure(size, 2, true, true)
        {}

        Buffer Weights;
        Buffer Pressure;
    };

    virtual void Init(Boundaries & boundaries) = 0;
    virtual Data & GetData() = 0;
    virtual void Solve() = 0;
};

}

#endif
