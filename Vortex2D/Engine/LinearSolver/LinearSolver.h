//
//  LinearSolver.h
//  Vortex2D
//

#ifndef Vortex2D_LinearSolver_h
#define Vortex2D_LinearSolver_h

#include "Operator.h"

namespace Vortex2D { namespace Fluid {

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

        /**
         * @brief Buffer with 4 components holding the weights of the 4 neighbouring grid cells
         */
        Buffer Weights;

        /**
         * @brief Buffer with 1 component holding the diagonal value of the matrix
         */
        Buffer Diagonal;

        /**
         * @brief Buffer with 2 components, first one is unknowns and second is the right hand side of the linear equations.
         */
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

}}

#endif
