//
//  LinearSolver.h
//  Vertex2D
//
//  Created by Maximilian Maldacker on 19/08/2015.
//  Copyright (c) 2015 Maximilian Maldacker. All rights reserved.
//

#ifndef Vertex2D_LinearSolver_h
#define Vertex2D_LinearSolver_h

#include "Operator.h"

namespace Fluid
{

/**
 * @brief An interface to represent a linear solver.
 */
struct LinearSolver
{
    /**
     * @brief The data of the linear solver.
     * In Ax = b,
     * x is first component of Pressure,
     * A is Weights + Diagonals,
     * b is second component of Pressure.
     */
    struct Data
    {
        Data(const glm::vec2 & size)
        : Weights(size, 4)
        , Diagonal(size, 1)
        , Pressure(size, 2, true, true)
        {}

        Buffer Weights;
        Buffer Diagonal;
        Buffer Pressure;
    };

    /**
     * @brief Any initialisation steps to be done before solving the linear equations
     */
    virtual void Init(Data & data) = 0;

    /**
     * @brief Solves the linear equations
     */
    virtual void Solve(Data & data) = 0;
};

}

#endif
