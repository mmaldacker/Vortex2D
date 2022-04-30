//
//  Pressure.cpp
//  Vortex
//

#include "Pressure.h"

#include <Vortex/Renderer/Pipeline.h>

#include "vortex_generated_spirv.h"

namespace Vortex
{
namespace Fluid
{
Pressure::Pressure(Renderer::Device& device,
                   float dt,
                   const glm::ivec2& size,
                   LinearSolver::Data& data,
                   Velocity& velocity,
                   Renderer::Texture& solidPhi,
                   Renderer::Texture& liquidPhi,
                   Renderer::GenericBuffer& valid)
    : mData(data)
    , mBuildMatrix(device, Renderer::ComputeSize{size}, SPIRV::BuildMatrix_comp)
    , mBuildMatrixBound(mBuildMatrix.Bind({data.Diagonal, data.Lower, liquidPhi, solidPhi}))
    , mBuildDiv(device, Renderer::ComputeSize{size}, SPIRV::BuildDiv_comp)
    , mBuildDivBound(mBuildDiv.Bind({data.B, data.Diagonal, liquidPhi, solidPhi, velocity}))
    , mProject(device, Renderer::ComputeSize{size}, SPIRV::Project_comp)
    , mProjectBound(
          mProject.Bind({data.X, liquidPhi, solidPhi, velocity, velocity.Output(), valid}))
    , mBuildEquationCmd(device, false)
    , mProjectCmd(device, false)
{
  mBuildEquationCmd.Record(
      [&](Renderer::CommandEncoder& command)
      {
        command.DebugMarkerBegin("Build equations", {0.02f, 0.68f, 0.84f, 1.0f});
        mBuildMatrixBound.PushConstant(command, dt);
        mBuildMatrixBound.Record(command);
        data.Diagonal.Barrier(command, Renderer::Access::Write, Renderer::Access::Read);
        data.Lower.Barrier(command, Renderer::Access::Write, Renderer::Access::Read);
        mBuildDivBound.Record(command);
        data.B.Barrier(command, Renderer::Access::Write, Renderer::Access::Read);
        command.DebugMarkerEnd();
      });

  mProjectCmd.Record(
      [&](Renderer::CommandEncoder& command)
      {
        command.DebugMarkerBegin("Pressure", {0.45f, 0.47f, 0.75f, 1.0f});
        valid.Clear(command);
        valid.Barrier(command, Renderer::Access::Write, Renderer::Access::Write);
        mProjectBound.PushConstant(command, dt);
        mProjectBound.Record(command);
        valid.Barrier(command, Renderer::Access::Write, Renderer::Access::Read);
        velocity.Output().Barrier(command,
                                  Renderer::ImageLayout::General,
                                  Renderer::Access::Write,
                                  Renderer::ImageLayout::General,
                                  Renderer::Access::Read);
        velocity.CopyBack(command);
        command.DebugMarkerEnd();
      });
}

Renderer::Work::Bound Pressure::BindMatrixBuild(const glm::ivec2& size,
                                                Renderer::GenericBuffer& diagonal,
                                                Renderer::GenericBuffer& lower,
                                                Renderer::Texture& liquidPhi,
                                                Renderer::Texture& solidPhi)
{
  return mBuildMatrix.Bind(Renderer::ComputeSize{size}, {diagonal, lower, liquidPhi, solidPhi});
}

void Pressure::BuildLinearEquation()
{
  mBuildEquationCmd.Submit();
}

void Pressure::ApplyPressure()
{
  mProjectCmd.Submit();
}

}  // namespace Fluid
}  // namespace Vortex
