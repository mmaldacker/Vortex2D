//
//  Multigrid.cpp
//  Vortex2D
//

#include "Multigrid.h"

namespace Vortex2D { namespace Fluid {
/*
Multigrid::Multigrid(glm::vec2 size)
    : mDepths(0)
    , mResidual(Renderer::Shader::TexturePositionVert, ResidualFrag)
    , mDampedJacobi(Renderer::Shader::TexturePositionVert, DampedJacobiFrag)
    , mIdentity(Renderer::Shader::TexturePositionVert, Renderer::Shader::TexturePositionFrag)
    , mScale(ScaleVert, ScaleFrag)
    , mBoundaryMask(Renderer::Shader::PositionVert, BoundaryMaskFrag)
{
    const float min_size = 4.0f;

    while (size.x > min_size && size.y > min_size)
    {
        size = glm::ceil(size/glm::vec2(2.0f));

        mDatas.emplace_back(size);

        mLiquidPhis.emplace_back(size, 1);
        mSolidPhis.emplace_back(glm::vec2(2)*size, 1);

        mSolidPhis.back().BorderColour(glm::vec4(-1.0f));
        mLiquidPhis.back().BorderColour(glm::vec4(-1.0f));

        mDepths++;
    }

    mGaussSeidel.SetW(1.0f);
}

void Multigrid::Smoother(Data& data, int iterations)
{
    for (int i = 0; i < iterations; i++)
    {
        data.Pressure.Swap();
        data.Pressure = mDampedJacobi(Back(data.Pressure), data.Weights, data.Diagonal);
    }
}

void Multigrid::BorderSmoother(Data& data, int iterations, bool up)
{
}

void Multigrid::Build(Renderer::Work& buildEquation,
                      Renderer::Buffer& solidPhi,
                      Renderer::Buffer& liquidPhi)
{
    data.Diagonal = diagonals(solidPhi, liquidPhi);
    data.Weights = weights(solidPhi, liquidPhi);

    mLiquidPhis[0] = mScale(liquidPhi);
    mSolidPhis[0] = mScale(solidPhi);

    for (int i = 1; i < mDepths; i++)
    {
        mLiquidPhis[i - 1].ClampToEdge();
        mLiquidPhis[i] = mScale(mLiquidPhis[i - 1]);

        mSolidPhis[i - 1].ClampToEdge();
        mSolidPhis[i] = mScale(mSolidPhis[i - 1]);
    }

    for (int i = 0; i < mDepths; i++)
    {
        auto& x = mDatas[i];

        mSolidPhis[i].ClampToBorder();
        mLiquidPhis[i].ClampToBorder();

        x.Diagonal = diagonals(mSolidPhis[i], mLiquidPhis[i]);
        x.Weights = weights(mSolidPhis[i], mLiquidPhis[i]);
    }
}

void Multigrid::Init(LinearSolver::Data& data)
{
    RenderMask(data.Pressure, data);
    RenderBoundaryMask(data, data.Diagonal);

    for (int i = 0; i < mDepths; i++)
    {
        //mDatas[i].Pressure.Clear(glm::vec4(0.0f));
        RenderMask(mDatas[i].Pressure, mDatas[i]);
        RenderBoundaryMask(mDatas[i], mDatas[i].Diagonal);
    }
}

void Multigrid::Solve(Parameters&)
{
    int numIterations = 2;

    Smoother(data, numIterations);
    //BorderSmoother(data, numIterations, true);

    if (mDepths > 0)
    {
        data.Pressure.Swap();
        data.Pressure = mResidual(Back(data.Pressure), data.Weights, data.Diagonal);

        mDatas[0].Pressure = mTransfer.Restrict(data.Pressure);

        for (int i = 0 ; i < mDepths - 1 ; i++)
        {
            numIterations *= 2;
            auto& x = mDatas[i];

            Smoother(x, numIterations);
            //BorderSmoother(x, numIterations, true);

            x.Pressure.Swap();
            x.Pressure = mResidual(Back(x.Pressure), x.Weights, x.Diagonal);

            mDatas[i + 1].Pressure = mTransfer.Restrict(x.Pressure);
        }

        Smoother(mDatas.back(), numIterations * 2);

        for (int i = mDepths - 2 ; i >= 0 ; --i)
        {
            auto& x = mDatas[i];

            x.Pressure.Swap();
            x.Pressure = mTransfer.Prolongate(mDatas[i + 1].Pressure, Back(x.Pressure));

            //BorderSmoother(x, numIterations, false);
            Smoother(x, numIterations);

            numIterations /= 2;
        }

        data.Pressure.Swap();
        data.Pressure = mTransfer.Prolongate(mDatas[0].Pressure, Back(data.Pressure));
    }

    //BorderSmoother(data, numIterations, false);
    Smoother(data, numIterations);
}
*/
}}
