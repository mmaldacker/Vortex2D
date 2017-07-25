//
//  Pressure.h
//  Vortex2D
//

#include <Vortex2D/Renderer/Buffer.h>

#include <Vortex2D/Engine/LinearSolver/LinearSolver.h>

namespace Vortex2D { namespace Fluid {

class Pressure
{
public:
    Pressure(float dt,
             const glm::vec2& size,
             LinearSolver& solver,
             LinearSolver::Data& data,
             Renderer::Buffer& velocity,
             Renderer::Buffer& solidPhi,
             Renderer::Buffer& liquidPhi,
             Renderer::Buffer& solidVelocity);

    void Solve(LinearSolver::Parameters& params);

private:

};

}}
