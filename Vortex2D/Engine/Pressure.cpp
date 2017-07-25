//
//  Pressure.cpp
//  Vortex2D
//

#include "Pressure.h"

namespace Vortex2D { namespace Fluid {

Pressure::Pressure(float dt,
                   const glm::vec2& size,
                   LinearSolver& solver,
                   LinearSolver::Data& data,
                   Renderer::Buffer& velocity,
                   Renderer::Buffer& solidPhi,
                   Renderer::Buffer& liquidPhi,
                   Renderer::Buffer& solidVelocity)
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
