//
//  LinearSolver.cpp
//  Vertex2D
//

#include "LinearSolver.h"

namespace Vortex2D { namespace Fluid {

LinearSolver::Parameters::Parameters(unsigned iterations, float errorTolerance)
    : Iterations(iterations)
    , ErrorTolerance(errorTolerance)
    , OutIterations(0)
    , OutError(0.0f)
{

}

bool LinearSolver::Parameters::IsFinished(unsigned iterations, float error) const
{
    if (Iterations > 0)
    {
        return iterations > Iterations  || error < ErrorTolerance;
    }
    else
    {
        return error < ErrorTolerance;
    }
}

}}
