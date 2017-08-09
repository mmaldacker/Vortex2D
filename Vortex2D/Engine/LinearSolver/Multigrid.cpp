//
//  Multigrid.cpp
//  Vortex2D
//

#include "Multigrid.h"

namespace Vortex2D { namespace Fluid {

namespace
{
const float min_size = 10.0f;
}

Multigrid::Multigrid(const Renderer::Device& device, glm::ivec2 size)
    : mSize(size)
    , mDepths(0)
    , mResidualWork(device, size, "../Vortex2D/Residual.comp.spv",
                    {vk::DescriptorType::eStorageBuffer,
                    vk::DescriptorType::eStorageBuffer,
                    vk::DescriptorType::eStorageBuffer,
                    vk::DescriptorType::eStorageBuffer})
    , mDampedJacobiWork(device, size, "../Vortex2D/DampedJacobi.comp.spv",
                        {vk::DescriptorType::eStorageBuffer,
                        vk::DescriptorType::eStorageBuffer,
                        vk::DescriptorType::eStorageBuffer,
                        vk::DescriptorType::eStorageBuffer})
    , mTransfer(device)
    , mCmd(device, false)
{
    while (size.x > min_size && size.y > min_size)
    {
        int width = size.x;
        int height = size.y;
        assert(width % 2 == 0);
        assert(height % 2 == 0);

        size = (size - glm::ivec2(2)) / glm::ivec2(2) + glm::ivec2(2);

        mMatrices.emplace_back(device, vk::BufferUsageFlagBits::eStorageBuffer, false, size.x*size.y*sizeof(Data));
        mPressures.emplace_back(device, vk::BufferUsageFlagBits::eStorageBuffer, false, size.x*size.y*sizeof(float));
        mResiduals.emplace_back(device, vk::BufferUsageFlagBits::eStorageBuffer, false, size.x*size.y*sizeof(float));
        mSolidPhis.emplace_back(device, vk::BufferUsageFlagBits::eStorageBuffer, false, size.x*size.y*sizeof(float));
        mLiquidPhis.emplace_back(device, vk::BufferUsageFlagBits::eStorageBuffer, false, size.x*size.y*sizeof(float));

        mDepths++;
    }
}

void Multigrid::Init(Renderer::Buffer& matrix, Renderer::Buffer& b, Renderer::Buffer& pressure)
{
    mResidualWorkBound.push_back(mResidualWork.Bind(mSize, {pressure, matrix, b, mResiduals[0]}));

    auto size = mSize;
    while (size.x > min_size && size.y > min_size)
    {

        size = (size - glm::ivec2(2)) / glm::ivec2(2) + glm::ivec2(2);

        //mTransfer.Init(size,        )
    };

    mCmd.Record([&](vk::CommandBuffer commandBuffer)
    {
        int numIterations = 4;
        int numBorderIterations = 2;

        pressure.Clear(commandBuffer);

        //Smoother(data, numIterations);
        //BorderSmoother(data, numBorderIterations, true);

        if (mDepths > 0)
        {
            for (int i = 0 ; i < mDepths - 1 ; i++)
            {
                numIterations *= 2;

                //Smoother(x, 4);
                //BorderSmoother(x, numBorderIterations, true);

                mResidualWorkBound[i].Record(commandBuffer);
                mResiduals[i].Barrier(commandBuffer, vk::AccessFlagBits::eShaderWrite, vk::AccessFlagBits::eShaderRead);
                mTransfer.Restrict(i);
                mPressures[i].Clear(commandBuffer);
            }

            //Smoother(mDatas.back(), numIterations * 2);

            for (int i = mDepths - 2 ; i >= 0 ; --i)
            {
                mTransfer.Prolongate(i);

                //BorderSmoother(x, numBorderIterations, false);
                //Smoother(x, numIterations);

                numIterations /= 2;
            }
        }

        //BorderSmoother(data, numBorderIterations, false);
        //Smoother(data, numIterations);
    });
}

/*
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
*/

void Multigrid::Solve(Parameters&)
{
    mCmd.Submit();
}

}}
