//
//  LinearSolver.cpp
//  Vertex2D
//

#include "LinearSolver.h"

namespace Vortex2D { namespace Fluid {

LinearSolver::Parameters::Parameters(SolverType type, unsigned iterations, float errorTolerance)
    : Type(type)
    , Iterations(iterations)
    , ErrorTolerance(errorTolerance)
    , OutIterations(0)
    , OutError(0.0f)
{

}

LinearSolver::Data::Data(const Renderer::Device& device, const glm::ivec2& size, VmaMemoryUsage memoryUsage)
    : Diagonal(device, size.x * size.y, memoryUsage)
    , Lower(device, size.x * size.y, memoryUsage)
    , B(device, size.x * size.y, memoryUsage)
    , X(device, size.x * size.y, memoryUsage)
{
}

bool LinearSolver::Parameters::IsFinished(float initialError) const
{
    if (Type == SolverType::Fixed)
    {
      return OutIterations > Iterations;
    }

    if (Iterations > 0)
    {
        return OutIterations >= Iterations  || OutError <= ErrorTolerance * initialError;
    }
    else
    {
        return OutError <= ErrorTolerance;
    }
}

void LinearSolver::Parameters::Reset()
{
    OutError = 0.0f;
    OutIterations = 0;
}

LinearSolver::Parameters FixedParams(unsigned iterations)
{
    return LinearSolver::Parameters(LinearSolver::Parameters::SolverType::Fixed, iterations);
}

LinearSolver::Parameters IterativeParams(float errorTolerance)
{
    return LinearSolver::Parameters(LinearSolver::Parameters::SolverType::Iterative, 1000, errorTolerance);
}

}}
