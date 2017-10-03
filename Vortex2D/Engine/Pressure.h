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
             LinearSolver::Data& data,
             Renderer::Texture& velocity,
             Renderer::Texture& solidPhi,
             Renderer::Texture& liquidPhi,
             Renderer::Texture& solidVelocity,
             Renderer::Buffer& valid);

    Renderer::Work::Bound BindMatrixBuild(const glm::ivec2& size,
                                          Renderer::Buffer& diagonal,
                                          Renderer::Buffer& lower,
                                          Renderer::Texture& liquidPhi,
                                          Renderer::Texture& solidPhi);

    void BuildLinearEquation();
    void ApplyPressure();


private:
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
