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

LinearSolver::Data::Data(const Renderer::Device& device, const glm::ivec2& size, bool host)
    : Diagonal(device, vk::BufferUsageFlagBits::eStorageBuffer, host,  size.x*size.y*sizeof(float))
    , Lower(device, vk::BufferUsageFlagBits::eStorageBuffer, host, size.x*size.y*sizeof(glm::vec2))
    , B(device, vk::BufferUsageFlagBits::eStorageBuffer, host, size.x*size.y*sizeof(float))
    , X(device, vk::BufferUsageFlagBits::eStorageBuffer, host, size.x*size.y*sizeof(float))
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
