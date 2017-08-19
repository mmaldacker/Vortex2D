//
//  LinearSolver.h
//  Vortex2D
//

#ifndef Vortex2D_LinearSolver_h
#define Vortex2D_LinearSolver_h

#include <Vortex2D/Renderer/Buffer.h>
#include <Vortex2D/Renderer/Texture.h>
#include <Vortex2D/Renderer/Work.h>

namespace Vortex2D { namespace Fluid {

/**
 * @brief An interface to represent a linear solver.
 */
struct LinearSolver
{
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

    virtual void Init(Renderer::Buffer& d,
                      Renderer::Buffer& l,
                      Renderer::Buffer& b,
                      Renderer::Buffer& x) = 0;

    /**
     * @brief Solves the linear equations
     */
    virtual void Solve(Parameters& params) = 0;
};

}}

#endif
