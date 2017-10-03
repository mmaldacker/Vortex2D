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
                   Renderer::Buffer& valid)
    : mBuildMatrix(device, size, "../Vortex2D/BuildMatrix.comp.spv",
{vk::DescriptorType::eStorageBuffer,
                   vk::DescriptorType::eStorageBuffer,
                   vk::DescriptorType::eStorageImage,
                   vk::DescriptorType::eStorageImage},
                   Renderer::PushConstantsSize<float>())
    , mBuildMatrixBound(mBuildMatrix.Bind({data.Diagonal,
                                           data.Lower,
                                           liquidPhi,
                                           solidPhi}))
    , mBuildDiv(device, size, "../Vortex2D/BuildDiv.comp.spv",
{vk::DescriptorType::eStorageBuffer,
                vk::DescriptorType::eStorageImage,
                vk::DescriptorType::eStorageImage,
                vk::DescriptorType::eStorageImage,
                vk::DescriptorType::eStorageImage})
    , mBuildDivBound(mBuildDiv.Bind({data.B,
                                     liquidPhi,
                                     solidPhi,
                                     velocity,
                                     solidVelocity}))
    , mProject(device, size, "../Vortex2D/Project.comp.spv",
{vk::DescriptorType::eStorageBuffer,
               vk::DescriptorType::eStorageImage,
               vk::DescriptorType::eStorageImage,
               vk::DescriptorType::eStorageImage,
               vk::DescriptorType::eStorageBuffer},
               Renderer::PushConstantsSize<float>())
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
                                                Renderer::Buffer& diagonal,
                                                Renderer::Buffer& lower,
                                                Renderer::Texture& liquidPhi,
                                                Renderer::Texture& solidPhi)
{
    return mBuildMatrix.Bind({diagonal, lower, liquidPhi, solidPhi});
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
