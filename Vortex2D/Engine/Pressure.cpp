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
                   LinearSolver& solver,
                   Renderer::Texture& velocity,
                   Renderer::Texture& solidPhi,
                   Renderer::Texture& liquidPhi,
                   Renderer::Texture& solidVelocity)
    : mSolver(solver)
    , mDiagonal(device,
            vk::BufferUsageFlagBits::eStorageBuffer,
            false,
            size.x*size.y*sizeof(float))
    , mLower(device,
            vk::BufferUsageFlagBits::eStorageBuffer,
            false,
            size.x*size.y*sizeof(glm::vec2))
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
                   vk::DescriptorType::eStorageBuffer,
                   vk::DescriptorType::eStorageImage,
                   vk::DescriptorType::eStorageImage},
                   Renderer::PushConstantsSize<float>())
    , mBuildMatrixBound(mBuildMatrix.Bind({mDiagonal,
                                           mLower,
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
               Renderer::PushConstantsSize<float>())
    , mProjectBound(mProject.Bind({mPressure, liquidPhi, solidPhi, velocity, solidVelocity}))
    , mBuildEquationCmd(device, false)
    , mProjectCmd(device, false)
{
    mBuildEquationCmd.Record([&](vk::CommandBuffer commandBuffer)
    {
        mBuildMatrixBound.PushConstant(commandBuffer, 8, dt);
        mBuildMatrixBound.Record(commandBuffer);
        mDiagonal.Barrier(commandBuffer, vk::AccessFlagBits::eShaderWrite, vk::AccessFlagBits::eShaderRead);
        mLower.Barrier(commandBuffer, vk::AccessFlagBits::eShaderWrite, vk::AccessFlagBits::eShaderRead);
        mBuildDivBound.Record(commandBuffer);
        mDiv.Barrier(commandBuffer, vk::AccessFlagBits::eShaderWrite, vk::AccessFlagBits::eShaderRead);
    });

    mProjectCmd.Record([&](vk::CommandBuffer commandBuffer)
    {
        // TODO need to set the valid buffer too
        mProjectBound.PushConstant(commandBuffer, 8, dt);
        mProjectBound.Record(commandBuffer);
        velocity.Barrier(commandBuffer,
                         vk::ImageLayout::eGeneral,
                         vk::AccessFlagBits::eShaderWrite,
                         vk::ImageLayout::eGeneral,
                         vk::AccessFlagBits::eShaderRead);
    });

    mSolver.Init(mDiagonal, mLower, mDiv, mPressure);
}

void Pressure::Solve(LinearSolver::Parameters& params)
{
    mBuildEquationCmd.Submit();
    mSolver.Solve(params);
    mProjectCmd.Submit();
}

}}
