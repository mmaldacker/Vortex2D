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
    : mData(device,
            vk::BufferUsageFlagBits::eStorageBuffer,
            false,
            size.x*size.y*sizeof(LinearSolver::Data))
    , mBuildEquationData(device, size, "../Vortex2D/BuildEquationData.comp.spv",
                        {vk::DescriptorType::eStorageBuffer,
                         vk::DescriptorType::eStorageImage,
                         vk::DescriptorType::eStorageImage,
                         vk::DescriptorType::eStorageImage,
                         vk::DescriptorType::eStorageImage})
    , mBuildEquationDataBound(mBuildEquationData.Bind({mData,
                                                       liquidPhi,
                                                       solidPhi,
                                                       velocity,
                                                       solidVelocity}))
{
}

void Pressure::Solve(LinearSolver::Parameters& params)
{
    /*
    mData.Pressure = mDiv(mVelocity, mSolidPhi, mLiquidPhi, mSolidVelocity);
    mData.Weights = mWeights(mSolidPhi, mLiquidPhi);
    mData.Diagonal = mDiagonals(mSolidPhi, mLiquidPhi);

    // TODO maybe move the two lines above inside Build? In any case this is not very clean
    mSolver.Build(mData, mDiagonals, mWeights, mSolidPhi, mLiquidPhi);
    mSolver.Init(mData);
    mSolver.Solve(mData, params);

    mVelocity.Swap();
    mVelocity = mProject(Back(mVelocity),
                         mData.Pressure,
                         mLiquidPhi,
                         mSolidPhi,
                         mSolidVelocity);
    */
}

}}
