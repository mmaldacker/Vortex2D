//
//  Pressure.h
//  Vortex2D
//

#include <Vortex2D/Renderer/Data.h>
#include <Vortex2D/Renderer/Operator.h>

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
    LinearSolver& mSolver;
    LinearSolver::Data& mData;
    Renderer::Buffer& mVelocity;
    Renderer::Buffer& mSolidPhi;
    Renderer::Buffer& mLiquidPhi;
    Renderer::Buffer& mSolidVelocity;

    Renderer::Operator mDiv;
    Renderer::Operator mProject;

    Renderer::Operator mWeights;
    Renderer::Operator mDiagonals;
};

}}
