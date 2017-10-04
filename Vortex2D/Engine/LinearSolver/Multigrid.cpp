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
    for (int i = 1; i <= mDepth.GetMaxDepth(); i++)
    {
        auto s = mDepth.GetDepthSize(i);

        mDiagonals.emplace_back(device, vk::BufferUsageFlagBits::eStorageBuffer, false, s.x*s.y*sizeof(float));
        mLowers.emplace_back(device, vk::BufferUsageFlagBits::eStorageBuffer, false, s.x*s.y*sizeof(glm::vec2));
        mPressures.emplace_back(device, vk::BufferUsageFlagBits::eStorageBuffer, false, s.x*s.y*sizeof(float));
        mBs.emplace_back(device, vk::BufferUsageFlagBits::eStorageBuffer, false, s.x*s.y*sizeof(float));

        mSolidPhis.emplace_back(device, s.x, s.y, vk::Format::eR32Sfloat, false);
        mLiquidPhis.emplace_back(device, s.x, s.y, vk::Format::eR32Sfloat, false);

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
    mDiagonal = &d;

    mResidualWorkBound.push_back(
                mResidualWork.Bind({pressure, d, l, b, mResiduals[0]}));

    mTransfer.InitRestrict(mDepth.GetDepthSize(0), mResiduals[0], mBs[0]);
    mTransfer.InitProlongate(mDepth.GetDepthSize(0), pressure, mPressures[0], d);

    // TODO init gauss seidel
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

void Multigrid::BuildRecursive(Pressure& pressure, int depth)
{
    auto s = mDepth.GetDepthSize(depth);

    if (depth < mDepth.GetMaxDepth())
    {
        mMatrixBuildBound.push_back(
                    pressure.BindMatrixBuild(s, mDiagonals[depth-1], mLowers[depth-1], mLiquidPhis[depth-1], mSolidPhis[depth-1]));

        mResidualWorkBound.push_back(
                    mResidualWork.Bind(s, {mPressures[depth-1], mDiagonals[depth-1], mLowers[depth-1], mBs[depth-1], mResiduals[depth]}));

        mTransfer.InitRestrict(s, mResiduals[depth], mBs[depth]);
        mTransfer.InitProlongate(s, mPressures[depth-1], mPressures[depth], mDiagonals[depth-1]);

        mSmoothers[depth].Init(mDiagonals[depth-1], mLowers[depth-1], mBs[depth-1], mPressures[depth-1]);
    }
    else
    {
        mMatrixBuildBound.push_back(pressure.BindMatrixBuild(s, mDiagonals[depth-1], mLowers[depth-1], mLiquidPhis[depth-1], mSolidPhis[depth-1]));
        mSmoothers[depth].Init(mDiagonals[depth-1], mLowers[depth-1], mBs[depth-1], mPressures[depth-1]);
    }
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
        mDiagonals[i].Barrier(commandBuffer, vk::AccessFlagBits::eShaderWrite, vk::AccessFlagBits::eShaderRead);
        mLowers[i].Barrier(commandBuffer, vk::AccessFlagBits::eShaderWrite, vk::AccessFlagBits::eShaderRead);
    }

    std::size_t maxDepth = mDepth.GetMaxDepth();
    mMatrixBuildBound[maxDepth - 1].PushConstant(commandBuffer, 8, mDelta);
    mMatrixBuildBound[maxDepth - 1].Record(commandBuffer);
    mDiagonals[maxDepth - 1].Barrier(commandBuffer, vk::AccessFlagBits::eShaderWrite, vk::AccessFlagBits::eShaderRead);
    mLowers[maxDepth - 1].Barrier(commandBuffer, vk::AccessFlagBits::eShaderWrite, vk::AccessFlagBits::eShaderRead);
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
        mPressures[i].Clear(commandBuffer);
    }

    Smoother(commandBuffer, mDepth.GetMaxDepth(), 50);

    for (int i = mDepth.GetMaxDepth() - 1; i >= 0; --i)
    {
        mTransfer.Prolongate(commandBuffer, i);
        Smoother(commandBuffer, i, numIterations);
    }
}


}}
