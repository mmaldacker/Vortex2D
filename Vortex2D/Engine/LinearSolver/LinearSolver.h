//
//  LinearSolver.h
//  Vortex2D
//

#ifndef Vortex2D_LinearSolver_h
#define Vortex2D_LinearSolver_h

#include <Vortex2D/Renderer/Operator.h>

#include <chrono>

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
        Data(const glm::vec2& size);

        /**
         * @brief Buffer with 4 components holding the weights of the 4 neighbouring grid cells
         */
        Renderer::Buffer Weights;

        /**
         * @brief Buffer with 1 component holding the diagonal value of the matrix
         */
        Renderer::Buffer Diagonal;

        /**
         * @brief Buffer with 2 components, first one is unknowns and second is the right hand side of the linear equations.
         */
        Renderer::Buffer Pressure;
    };

    struct Parameters
    {
        Parameters(unsigned iterations, float errorTolerance = 0.0f);

        bool IsFinished(unsigned iterations, float error) const;

        unsigned Iterations;

        float ErrorTolerance;

        unsigned OutIterations;

        std::chrono::microseconds initTime;
        std::chrono::microseconds solveTime;
    };

    LinearSolver();

    virtual ~LinearSolver() {}

    virtual void Build(Data& data,
                       Renderer::Operator& diagonals,
                       Renderer::Operator& weights,
                       Renderer::Buffer& solidPhi,
                       Renderer::Buffer& liquidPhi) = 0;

    /**
     * @brief Any initialisation steps to be done before solving the linear equations
     */
    virtual void Init(Data& data) = 0;

    /**
     * @brief Solves the linear equations
     */
    virtual void Solve(Data& data, Parameters& params) = 0;

    /**
     * @brief Renders a stencil using the diagonal, to avoid division by 0 in the solvers
     * @param Buffer to update the stencil
     * @param Diagonal will be used for the mask
     */
    void RenderMask(Renderer::Buffer& destination, Data& data);

private:
    Renderer::Operator mMask;
};

}}

#endif
