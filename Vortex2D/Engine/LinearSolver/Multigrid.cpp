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
        assert(s.x % 2 == 0);
        assert(s.y % 2 == 0);

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

Multigrid::Multigrid(const Renderer::Device& device, const glm::ivec2& size, float delta)
    : mDepth(size)
    , mDelta(delta)
    , mResidualWork(device, size, "../Vortex2D/Residual.comp.spv")
    , mTransfer(device)
    , mPhiScaleWork(device, size, "../Vortex2D/PhiScale.comp.spv")
{
    mSmoothers.emplace_back(device, size);

    for (int i = 1; i <= mDepth.GetMaxDepth(); i++)
    {
        auto s = mDepth.GetDepthSize(i);
        mDatas.emplace_back(device, s);

        mSolidPhis.emplace_back(device, s.x, s.y, vk::Format::eR32Sfloat, false);
        mLiquidPhis.emplace_back(device, s.x, s.y, vk::Format::eR32Sfloat, false);

        mSmoothers.emplace_back(device, s);

        ExecuteCommand(device, [&](vk::CommandBuffer commandBuffer)
        {
            mSolidPhis.back().Clear(commandBuffer, std::array<float, 4>{{-1.0f, 0.0f, 0.0f, 0.0f}});
            mLiquidPhis.back().Clear(commandBuffer, std::array<float, 4>{{1.0f, 0.0f, 0.0f, 0.0f}});
        });
    }

    for (int i = 0; i < mDepth.GetMaxDepth(); i++)
    {
        auto s = mDepth.GetDepthSize(i);
        mResiduals.emplace_back(device, vk::BufferUsageFlagBits::eStorageBuffer, false, s.x*s.y*sizeof(float));
    }
}

void Multigrid::Init(Renderer::Buffer& d,
                     Renderer::Buffer& l,
                     Renderer::Buffer& b,
                     Renderer::Buffer& pressure)

{
    mPressure = &pressure;

    mResidualWorkBound.push_back(
                mResidualWork.Bind({pressure, d, l, b, mResiduals[0]}));

    auto s = mDepth.GetDepthSize(0);
    mTransfer.InitRestrict(s, mResiduals[0], mDatas[0].B);
    mTransfer.InitProlongate(s, pressure, mDatas[0].X, d);

    mSmoothers[0].Init(d, l, b, pressure);
}

void Multigrid::Build(Pressure& pressure,
                      Renderer::Texture& solidPhi,
                      Renderer::Texture& liquidPhi)
{
    auto s = mDepth.GetDepthSize(1);
    mLiquidPhiScaleWorkBound.push_back(mPhiScaleWork.Bind(s, {liquidPhi, mLiquidPhis[0]}));
    mSolidPhiScaleWorkBound.push_back(mPhiScaleWork.Bind(s, {solidPhi, mSolidPhis[0]}));

    BuildRecursive(pressure, 1);
}

void Multigrid::BuildRecursive(Pressure& pressure, std::size_t depth)
{
    auto s = mDepth.GetDepthSize(depth);

    if (depth < mDepth.GetMaxDepth())
    {
        mLiquidPhiScaleWorkBound.push_back(mPhiScaleWork.Bind(s, {mLiquidPhis[depth - 1], mLiquidPhis[depth]}));
        mSolidPhiScaleWorkBound.push_back(mPhiScaleWork.Bind(s, {mSolidPhis[depth - 1], mSolidPhis[depth]}));

        mResidualWorkBound.push_back(
                    mResidualWork.Bind(s, {mDatas[depth-1].X,
                                           mDatas[depth-1].Diagonal,
                                           mDatas[depth-1].Lower,
                                           mDatas[depth-1].B,
                                           mResiduals[depth]}));

        mTransfer.InitRestrict(s,
                               mResiduals[depth],
                               mDatas[depth].B);

        mTransfer.InitProlongate(s,
                                 mDatas[depth-1].X,
                                 mDatas[depth].X,
                                 mDatas[depth-1].Diagonal);

        BuildRecursive(pressure, depth+1);
    }

    mMatrixBuildBound.push_back(
                pressure.BindMatrixBuild(s,
                                         mDatas[depth-1].Diagonal,
                                         mDatas[depth-1].Lower,
                                         mLiquidPhis[depth-1],
                                         mSolidPhis[depth-1]));


    mSmoothers[depth].Init(mDatas[depth-1].Diagonal,
                           mDatas[depth-1].Lower,
                           mDatas[depth-1].B,
                           mDatas[depth-1].X);
}

void Multigrid::Smoother(vk::CommandBuffer commandBuffer, int n, int iterations)
{
    mSmoothers[n].SetPreconditionerIterations(iterations);
    mSmoothers[n].SetW(1.0);
    mSmoothers[n].Record(commandBuffer);
}

void Multigrid::RecordInit(vk::CommandBuffer commandBuffer)
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
}

void Multigrid::Record(vk::CommandBuffer commandBuffer)
{
    const int numIterations = 2;

    assert(mPressure != nullptr);
    mPressure->Clear(commandBuffer);

    for (int i = 0; i < mDepth.GetMaxDepth(); i++)
    {
        Smoother(commandBuffer, i, numIterations);

        mResidualWorkBound[i].Record(commandBuffer);
        mResiduals[i].Barrier(commandBuffer, vk::AccessFlagBits::eShaderWrite, vk::AccessFlagBits::eShaderRead);
        mTransfer.Restrict(commandBuffer, i);
        mDatas[i].X.Clear(commandBuffer);
    }

    Smoother(commandBuffer, mDepth.GetMaxDepth(), 50);

    for (int i = mDepth.GetMaxDepth() - 1; i >= 0; --i)
    {
        mTransfer.Prolongate(commandBuffer, i);
        Smoother(commandBuffer, i, numIterations);
    }
}


}}
