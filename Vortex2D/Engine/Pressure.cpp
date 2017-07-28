//
//  Pressure.cpp
//  Vortex2D
//

#include "Pressure.h"

namespace Vortex2D { namespace Fluid {

Pressure::Pressure(const Renderer::Device& device,
                   float dt,
                   const glm::vec2& size,
                   LinearSolver& solver,
                   Renderer::Texture& velocity,
                   Renderer::Texture& solidPhi,
                   Renderer::Texture& liquidPhi,
                   Renderer::Texture& solidVelocity)
    : mSolver(solver)
    , mData(device,
            vk::BufferUsageFlagBits::eStorageBuffer,
            false,
            size.x*size.y*sizeof(LinearSolver::Data))
    , mPressure(device,
                vk::BufferUsageFlagBits::eStorageBuffer,
                false,
                size.x*size.y*sizeof(float))
    , mBuildEquationData(device, size, "../Vortex2D/BuildEquationData.comp.spv",
                        {vk::DescriptorType::eStorageBuffer,
                         vk::DescriptorType::eStorageImage,
                         vk::DescriptorType::eStorageImage,
                         vk::DescriptorType::eStorageImage,
                         vk::DescriptorType::eStorageImage},
                         4)
    , mBuildEquationDataBound(mBuildEquationData.Bind({mData,
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
    , mBuildEquationCmd(device)
    , mProjectCmd(device)
{
    mBuildEquationCmd.Record([&](vk::CommandBuffer commandBuffer)
    {
        // TODO barrier for liquidPhi, solidPhi, velocity and solidVelocity
        mBuildEquationDataBound.PushConstant(commandBuffer, 8, dt);
        mBuildEquationDataBound.Record(commandBuffer);
        mData.Barrier(commandBuffer, vk::AccessFlagBits::eShaderWrite, vk::AccessFlagBits::eShaderRead);
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

    mSolver.Init(mData, mPressure);
}

void Pressure::Solve(LinearSolver::Parameters& params)
{
    mBuildEquationCmd.Submit();
    mSolver.Solve(params);
    mProjectCmd.Submit();
}

}}
