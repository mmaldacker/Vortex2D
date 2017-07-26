//
//  Pressure.h
//  Vortex2D
//

#include <Vortex2D/Renderer/Buffer.h>
#include <Vortex2D/Renderer/Texture.h>
#include <Vortex2D/Renderer/Work.h>
#include <Vortex2D/Engine/LinearSolver/LinearSolver.h>

namespace Vortex2D { namespace Fluid {

class Pressure
{
public:
    Pressure(const Renderer::Device& device,
             float dt,
             const glm::vec2& size,
             LinearSolver& solver,
             Renderer::Texture& velocity,
             Renderer::Texture& solidPhi,
             Renderer::Texture& liquidPhi,
             Renderer::Texture& solidVelocity);

    void Solve(LinearSolver::Parameters& params);

private:
    Renderer::Buffer mData;
    Renderer::Work mBuildEquationData;
    Renderer::Work::Bound mBuildEquationDataBound;
};

}}
