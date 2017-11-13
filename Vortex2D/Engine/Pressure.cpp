//
//  Pressure.cpp
//  Vortex2D
//

#include "Pressure.h"

#include <Vortex2D/Renderer/Pipeline.h>

namespace Vortex2D { namespace Fluid {

Pressure::Pressure(const Renderer::Device& device,
                   float dt,
                   const glm::ivec2& size,
                   LinearSolver::Data& data,
                   Renderer::Texture& velocity,
                   Renderer::Texture& solidPhi,
                   Renderer::Texture& liquidPhi,
                   Renderer::Texture& solidVelocity,
                   Renderer::GenericBuffer& valid)
    : mBuildMatrix(device, size, "../Vortex2D/BuildMatrix.comp.spv")
    , mBuildMatrixBound(mBuildMatrix.Bind({data.Diagonal,
                                           data.Lower,
                                           liquidPhi,
                                           solidPhi}))
    , mBuildDiv(device, size, "../Vortex2D/BuildDiv.comp.spv")
    , mBuildDivBound(mBuildDiv.Bind({data.B,
                                     data.Diagonal,
                                     liquidPhi,
                                     solidPhi,
                                     velocity,
                                     solidVelocity}))
    , mProject(device, size, "../Vortex2D/Project.comp.spv")
    , mProjectBound(mProject.Bind({data.X, liquidPhi, solidPhi, velocity, valid}))
    , mBuildEquationCmd(device, false)
    , mProjectCmd(device, false)
{
    mBuildEquationCmd.Record([&](vk::CommandBuffer commandBuffer)
    {
        mBuildMatrixBound.PushConstant(commandBuffer, 8, dt);
        mBuildMatrixBound.Record(commandBuffer);
        data.Diagonal.Barrier(commandBuffer, vk::AccessFlagBits::eShaderWrite, vk::AccessFlagBits::eShaderRead);
        data.Lower.Barrier(commandBuffer, vk::AccessFlagBits::eShaderWrite, vk::AccessFlagBits::eShaderRead);
        mBuildDivBound.Record(commandBuffer);
        data.B.Barrier(commandBuffer, vk::AccessFlagBits::eShaderWrite, vk::AccessFlagBits::eShaderRead);
    });

    mProjectCmd.Record([&](vk::CommandBuffer commandBuffer)
    {
        valid.Clear(commandBuffer);
        mProjectBound.PushConstant(commandBuffer, 8, dt);
        mProjectBound.Record(commandBuffer);
        velocity.Barrier(commandBuffer,
                         vk::ImageLayout::eGeneral, vk::AccessFlagBits::eShaderWrite,
                         vk::ImageLayout::eGeneral, vk::AccessFlagBits::eShaderRead);
    });

}

Renderer::Work::Bound Pressure::BindMatrixBuild(const glm::ivec2& size,
                                                Renderer::GenericBuffer& diagonal,
                                                Renderer::GenericBuffer& lower,
                                                Renderer::Texture& liquidPhi,
                                                Renderer::Texture& solidPhi)
{
    return mBuildMatrix.Bind(size, {diagonal, lower, liquidPhi, solidPhi});
}

void Pressure::BuildLinearEquation()
{
    mBuildEquationCmd.Submit();
}

void Pressure::ApplyPressure()
{
    mProjectCmd.Submit();
}

}}
