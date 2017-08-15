//
//  Pressure.cpp
//  Vortex2D
//

#include "Pressure.h"

namespace Vortex2D { namespace Fluid {

Pressure::Pressure(const Renderer::Device& device,
                   float dt,
                   const glm::ivec2& size,
                   LinearSolver& solver,
                   Renderer::Texture& velocity,
                   Renderer::Texture& solidPhi,
                   Renderer::Texture& liquidPhi,
                   Renderer::Texture& solidVelocity)
    : mSolver(solver)
    , mMatrix(device,
            vk::BufferUsageFlagBits::eStorageBuffer,
            false,
            size.x*size.y*sizeof(LinearSolver::Data))
    , mDiv(device,
           vk::BufferUsageFlagBits::eStorageBuffer,
           false,
           size.x*size.y*sizeof(float))
    , mPressure(device,
                vk::BufferUsageFlagBits::eStorageBuffer,
                false,
                size.x*size.y*sizeof(float))
    , mBuildMatrix(device, size, "../Vortex2D/BuildMatrix.comp.spv",
                   {vk::DescriptorType::eStorageBuffer,
                   vk::DescriptorType::eStorageImage,
                   vk::DescriptorType::eStorageImage},
                   4)
    , mBuildMatrixBound(mBuildMatrix.Bind({mMatrix,
                                           liquidPhi,
                                           solidPhi}))
    , mBuildDiv(device, size, "../Vortex2D/BuildDiv.comp.spv",
                {vk::DescriptorType::eStorageBuffer,
                vk::DescriptorType::eStorageImage,
                vk::DescriptorType::eStorageImage,
                vk::DescriptorType::eStorageImage,
                vk::DescriptorType::eStorageImage})
    , mBuildDivBound(mBuildDiv.Bind({mDiv,
                                     liquidPhi,
                                     solidPhi,
                                     velocity,
                                     solidVelocity}))
    , mProject(device, size, "../Vortex2D/Project.comp.spv",
              {vk::DescriptorType::eStorageBuffer,
               vk::DescriptorType::eStorageImage,
               vk::DescriptorType::eStorageImage,
               vk::DescriptorType::eStorageImage,
               vk::DescriptorType::eStorageImage},
               4)
    , mProjectBound(mProject.Bind({mPressure, liquidPhi, solidPhi, velocity, solidVelocity}))
    , mBuildEquationCmd(device, false)
    , mProjectCmd(device, false)
{
    mBuildEquationCmd.Record([&](vk::CommandBuffer commandBuffer)
    {
        mBuildMatrixBound.PushConstant(commandBuffer, 8, dt);
        mBuildMatrixBound.Record(commandBuffer);
        mMatrix.Barrier(commandBuffer, vk::AccessFlagBits::eShaderWrite, vk::AccessFlagBits::eShaderRead);
        mBuildDivBound.Record(commandBuffer);
        mDiv.Barrier(commandBuffer, vk::AccessFlagBits::eShaderWrite, vk::AccessFlagBits::eShaderRead);
    });

    mProjectCmd.Record([&](vk::CommandBuffer commandBuffer)
    {
        // TODO barrier for pressure
        mProjectBound.PushConstant(commandBuffer, 8, dt);
        mProjectBound.Record(commandBuffer);
        velocity.Barrier(commandBuffer,
                         vk::ImageLayout::eGeneral,
                         vk::AccessFlagBits::eShaderWrite,
                         vk::ImageLayout::eGeneral,
                         vk::AccessFlagBits::eShaderRead);
    });

    mSolver.Init(mMatrix, mDiv, mPressure);
}

void Pressure::Solve(LinearSolver::Parameters& params)
{
    mBuildEquationCmd.Submit();
    mSolver.Solve(params);
    mProjectCmd.Submit();
}

}}
