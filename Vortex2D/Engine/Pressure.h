//
//  Pressure.h
//  Vortex2D
//

#ifndef Vortex2d_Pressure_h
#define Vortex2d_Pressure_h

#include <Vortex2D/Renderer/Buffer.h>
#include <Vortex2D/Renderer/Texture.h>
#include <Vortex2D/Renderer/Work.h>
#include <Vortex2D/Renderer/CommandBuffer.h>
#include <Vortex2D/Engine/LinearSolver/LinearSolver.h>
#include <Vortex2D/Engine/Velocity.h>
#include <Vortex2D/Engine/Rigidbody.h>

namespace Vortex2D { namespace Fluid {

/**
 * @brief build the linear equation and compute the divergence from the resulting solution.
 */
class Pressure
{
public:
    Pressure(const Renderer::Device& device,
             float dt,
             const glm::ivec2& size,
             LinearSolver::Data& data,
             Velocity& velocity,
             Renderer::Texture& solidPhi,
             Renderer::Texture& liquidPhi,
             Renderer::GenericBuffer& valid);

    Renderer::Work::Bound BindMatrixBuild(const glm::ivec2& size,
                                          Renderer::GenericBuffer& diagonal,
                                          Renderer::GenericBuffer& lower,
                                          Renderer::Texture& liquidPhi,
                                          Renderer::Texture& solidPhi);

    void BuildLinearEquation();
    void ApplyPressure();

private:
    LinearSolver::Data& mData;
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

#endif
