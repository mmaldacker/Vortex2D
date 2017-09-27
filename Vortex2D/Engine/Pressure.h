//
//  Pressure.h
//  Vortex2D
//

#include <Vortex2D/Renderer/Buffer.h>
#include <Vortex2D/Renderer/Texture.h>
#include <Vortex2D/Renderer/Work.h>
#include <Vortex2D/Renderer/CommandBuffer.h>
#include <Vortex2D/Engine/LinearSolver/LinearSolver.h>

namespace Vortex2D { namespace Fluid {

class Pressure
{
public:
    Pressure(const Renderer::Device& device,
             float dt,
             const glm::ivec2& size,
             LinearSolver& solver,
             Renderer::Texture& velocity,
             Renderer::Texture& solidPhi,
             Renderer::Texture& liquidPhi,
             Renderer::Texture& solidVelocity,
             Renderer::Buffer& valid);

    void Solve(LinearSolver::Parameters& params);

private:
    LinearSolver& mSolver;
    Renderer::Buffer mDiagonal;
    Renderer::Buffer mLower;
    Renderer::Buffer mDiv;
    Renderer::Buffer mPressure;
    Renderer::Work mBuildMatrix;
    Renderer::Work::Bound mBuildMatrixBound;
    Renderer::Work mBuildDiv;
    Renderer::Work::Bound mBuildDivBound;
    Renderer::Work mProject;
    Renderer::Work::Bound mProjectBound;
    Renderer::CommandBuffer mBuildEquationCmd;
    Renderer::CommandBuffer mProjectCmd;
};

}}
