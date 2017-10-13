//
//  Multigrid.cpp
//  Vortex2D
//

#include "Multigrid.h"

#include <Vortex2D/Engine/Pressure.h>

namespace Vortex2D { namespace Fluid {

Depth::Depth(const glm::ivec2& size)
{
    auto s = size;
    mDepths.push_back(s);

    const float min_size = 10.0f;
    while (s.x > min_size && s.y > min_size)
    {
        if (s.x % 2 != 0 || s.y % 2 != 0) throw std::runtime_error("Invalid multigrid size");

        s = (s - glm::ivec2(2)) / glm::ivec2(2) + glm::ivec2(2);
        mDepths.push_back(s);
    }
}

int Depth::GetMaxDepth() const
{
    return mDepths.size() - 1;
}

glm::ivec2 Depth::GetDepthSize(int i) const
{
    assert(i < mDepths.size());
    return mDepths[i];
}

Multigrid::Multigrid(const Renderer::Device& device, const glm::ivec2& size, float delta, bool statistics)
    : mDepth(size)
    , mDelta(delta)
    , mResidualWork(device, size, "../Vortex2D/Residual.comp.spv")
    , mTransfer(device)
    , mPhiScaleWork(device, size, "../Vortex2D/PhiScale.comp.spv")
    , mBuildHierarchies(device)
    , mEnableStatistics(statistics)
    , mStatistics(device)
{
    mSmoothers.emplace_back(device, size);

    for (int i = 1; i <= mDepth.GetMaxDepth(); i++)
    {
        auto s = mDepth.GetDepthSize(i);
        mDatas.emplace_back(device, s);

        mSolidPhis.emplace_back(device, s);
        mLiquidPhis.emplace_back(device, s);

        mLiquidPhis.back().ExtrapolateInit(mSolidPhis.back());

        mSmoothers.emplace_back(device, s);

        ExecuteCommand(device, [&](vk::CommandBuffer commandBuffer)
        {
            mSolidPhis.back().Clear(commandBuffer, std::array<float, 4>{{-0.5f, 0.0f, 0.0f, 0.0f}});
            mLiquidPhis.back().Clear(commandBuffer, std::array<float, 4>{{0.5f, 0.0f, 0.0f, 0.0f}});
        });
    }

    for (int i = 0; i < mDepth.GetMaxDepth(); i++)
    {
        auto s = mDepth.GetDepthSize(i);
        mResiduals.emplace_back(device, s.x*s.y);
    }
}

void Multigrid::Init(Renderer::GenericBuffer& d,
                     Renderer::GenericBuffer& l,
                     Renderer::GenericBuffer& b,
                     Renderer::GenericBuffer& pressure)

{
    mPressure = &pressure;

    mResidualWorkBound.push_back(
                mResidualWork.Bind({pressure, d, l, b, mResiduals[0]}));

    auto s = mDepth.GetDepthSize(0);
    mTransfer.InitRestrict(0, s, mResiduals[0], d, mDatas[0].B, mDatas[0].Diagonal);
    mTransfer.InitProlongate(0, s, pressure, d, mDatas[0].X, mDatas[0].Diagonal);

    mSmoothers[0].Init(d, l, b, pressure);
}

void Multigrid::BuildHierarchiesInit(Pressure& pressure,
                                     Renderer::Texture& solidPhi,
                                     Renderer::Texture& liquidPhi)
{
    auto s = mDepth.GetDepthSize(1);
    mLiquidPhiScaleWorkBound.push_back(mPhiScaleWork.Bind(s, {liquidPhi, mLiquidPhis[0]}));
    mSolidPhiScaleWorkBound.push_back(mPhiScaleWork.Bind(s, {solidPhi, mSolidPhis[0]}));

    BindRecursive(pressure, 1);

    mBuildHierarchies.Record([&](vk::CommandBuffer commandBuffer)
    {
        for (std::size_t i = 0; i < mDepth.GetMaxDepth(); i++)
        {
            mLiquidPhiScaleWorkBound[i].Record(commandBuffer);
            mLiquidPhis[i].Barrier(commandBuffer,
                                   vk::ImageLayout::eGeneral,
                                   vk::AccessFlagBits::eShaderWrite,
                                   vk::ImageLayout::eGeneral,
                                   vk::AccessFlagBits::eShaderRead);

            mSolidPhiScaleWorkBound[i].Record(commandBuffer);
            mSolidPhis[i].Barrier(commandBuffer,
                                  vk::ImageLayout::eGeneral,
                                  vk::AccessFlagBits::eShaderWrite,
                                  vk::ImageLayout::eGeneral,
                                  vk::AccessFlagBits::eShaderRead);

            mLiquidPhis[i].ExtrapolateRecord(commandBuffer);

            mMatrixBuildBound[i].PushConstant(commandBuffer, 8, mDelta);
            mMatrixBuildBound[i].Record(commandBuffer);
            mDatas[i].Diagonal.Barrier(commandBuffer, vk::AccessFlagBits::eShaderWrite, vk::AccessFlagBits::eShaderRead);
            mDatas[i].Lower.Barrier(commandBuffer, vk::AccessFlagBits::eShaderWrite, vk::AccessFlagBits::eShaderRead);
        }

        std::size_t maxDepth = mDepth.GetMaxDepth();
        mMatrixBuildBound[maxDepth - 1].PushConstant(commandBuffer, 8, mDelta);
        mMatrixBuildBound[maxDepth - 1].Record(commandBuffer);
        mDatas[maxDepth - 1].Diagonal.Barrier(commandBuffer, vk::AccessFlagBits::eShaderWrite, vk::AccessFlagBits::eShaderRead);
        mDatas[maxDepth - 1].Lower.Barrier(commandBuffer, vk::AccessFlagBits::eShaderWrite, vk::AccessFlagBits::eShaderRead);
    });
}

void Multigrid::BindRecursive(Pressure& pressure, std::size_t depth)
{
    auto s0 = mDepth.GetDepthSize(depth);

    if (depth < mDepth.GetMaxDepth())
    {
        auto s1 = mDepth.GetDepthSize(depth + 1);
        mLiquidPhiScaleWorkBound.push_back(mPhiScaleWork.Bind(s1, {mLiquidPhis[depth - 1], mLiquidPhis[depth]}));
        mSolidPhiScaleWorkBound.push_back(mPhiScaleWork.Bind(s1, {mSolidPhis[depth - 1], mSolidPhis[depth]}));

        mResidualWorkBound.push_back(
                    mResidualWork.Bind(s0, {mDatas[depth-1].X,
                                           mDatas[depth-1].Diagonal,
                                           mDatas[depth-1].Lower,
                                           mDatas[depth-1].B,
                                           mResiduals[depth]}));

        mTransfer.InitRestrict(depth, s0,
                               mResiduals[depth],
                               mDatas[depth-1].Diagonal,
                               mDatas[depth].B,
                               mDatas[depth].Diagonal);

        mTransfer.InitProlongate(depth, s0,
                                 mDatas[depth-1].X,
                                 mDatas[depth-1].Diagonal,
                                 mDatas[depth].X,
                                 mDatas[depth].Diagonal);

        BindRecursive(pressure, depth+1);
    }

    mMatrixBuildBound.push_back(
                pressure.BindMatrixBuild(s0,
                                         mDatas[depth-1].Diagonal,
                                         mDatas[depth-1].Lower,
                                         mLiquidPhis[depth-1],
                                         mSolidPhis[depth-1]));


    mSmoothers[depth].Init(mDatas[depth-1].Diagonal,
                           mDatas[depth-1].Lower,
                           mDatas[depth-1].B,
                           mDatas[depth-1].X);
}

void Multigrid::BuildHierarchies()
{
    mBuildHierarchies.Submit();
}

void Multigrid::Smoother(vk::CommandBuffer commandBuffer, int n, int iterations)
{
    mSmoothers[n].SetPreconditionerIterations(iterations);
    mSmoothers[n].SetW(1.0);
    mSmoothers[n].Record(commandBuffer);
}

void Multigrid::Record(vk::CommandBuffer commandBuffer)
{
    const int numIterations = 4;

    if (mEnableStatistics) mStatistics.Start(commandBuffer);

    assert(mPressure != nullptr);
    mPressure->Clear(commandBuffer);

    if (mEnableStatistics) mStatistics.Tick(commandBuffer, "clear");

    for (int i = 0; i < mDepth.GetMaxDepth(); i++)
    {
        Smoother(commandBuffer, i, numIterations);
        if (mEnableStatistics) mStatistics.Tick(commandBuffer, "smoother " + std::to_string(i));

        mResidualWorkBound[i].Record(commandBuffer);
        mResiduals[i].Barrier(commandBuffer, vk::AccessFlagBits::eShaderWrite, vk::AccessFlagBits::eShaderRead);
        if (mEnableStatistics) mStatistics.Tick(commandBuffer, "residual " + std::to_string(i));

        mTransfer.Restrict(commandBuffer, i);
        if (mEnableStatistics) mStatistics.Tick(commandBuffer, "transfer " + std::to_string(i));

        mDatas[i].X.Clear(commandBuffer);
        if (mEnableStatistics) mStatistics.Tick(commandBuffer, "clear " + std::to_string(i));

    }

    Smoother(commandBuffer, mDepth.GetMaxDepth(), 100);
    if (mEnableStatistics) mStatistics.Tick(commandBuffer, "smoother max");


    for (int i = mDepth.GetMaxDepth() - 1; i >= 0; --i)
    {
        mTransfer.Prolongate(commandBuffer, i);
        if (mEnableStatistics) mStatistics.Tick(commandBuffer, "prolongate " + std::to_string(i));

        Smoother(commandBuffer, i, numIterations);
        if (mEnableStatistics) mStatistics.Tick(commandBuffer, "smoother " + std::to_string(i));
    }

    if (mEnableStatistics) mStatistics.End(commandBuffer, "end");
}

Renderer::Statistics::Timestamps Multigrid::GetStatistics()
{
    return mStatistics.GetTimestamps();
}

}}
