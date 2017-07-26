//
//  LinearSolver.h
//  Vortex2D
//

#ifndef Vortex2D_LinearSolver_h
#define Vortex2D_LinearSolver_h

#include <Vortex2D/Renderer/Buffer.h>

namespace Vortex2D { namespace Fluid {

/**
 * @brief An interface to represent a linear solver.
 */
struct LinearSolver
{
    /**
     * @brief The data of the linear solver.
     * In Ax = b,
     * A is Weights + Diagonals,
     * b is Div
     */
    struct Data
    {
        alignas(16) glm::vec4 Weights;
        alignas(4)  float Diagonal;
        alignas(4)  float Div;
    };

    struct Parameters
    {
        Parameters(unsigned iterations, float errorTolerance = 0.0f);

        bool IsFinished(unsigned iterations, float error) const;

        unsigned Iterations;
        float ErrorTolerance;
        unsigned OutIterations;
        float OutError;
    };

    virtual ~LinearSolver() {}

    /**
     * @brief Solves the linear equations
     */
    virtual void Solve(Renderer::Buffer& pressure, Renderer::Buffer& data, Parameters& params) = 0;
};

}}

#endif
