//
//  Multigrid.cpp
//  Vortex2D
//

#include "Multigrid.h"

#include <Vortex2D/Engine/Pressure.h>

#include "vortex2d_generated_spirv.h"

namespace Vortex2D { namespace Fluid {

Depth::Depth(const glm::ivec2& size)
{
    auto s = size;
    mDepths.push_back(s);

    const float min_size = 16.0f;
    while (s.x > min_size && s.y > min_size)
    {
        if (s.x % 2 != 0 || s.y % 2 != 0) throw std::runtime_error("Invalid multigrid size");

        s = s / glm::ivec2(2);
        mDepths.push_back(s);
    }
}

int Depth::GetMaxDepth() const
{
    return static_cast<int>(mDepths.size() - 1);
}

glm::ivec2 Depth::GetDepthSize(std::size_t i) const
{
    assert(i < mDepths.size());
    return mDepths[i];
}

Multigrid::Multigrid(const Renderer::Device& device, const glm::ivec2& size, float delta)
    : mDepth(size)
    , mDelta(delta)
    , mResidualWork(device, size, SPIRV::Residual_comp)
    , mTransfer(device)
    , mPhiScaleWork(device, size, SPIRV::PhiScale_comp)
    , mSmoother(device, mDepth.GetDepthSize(mDepth.GetMaxDepth()))
    , mBuildHierarchies(device, false)
{
    for (int i = 1; i <= mDepth.GetMaxDepth(); i++)
    {
        auto s = mDepth.GetDepthSize(i);
        mDatas.emplace_back(device, s);

        mSolidPhis.emplace_back(device, s);
        mLiquidPhis.emplace_back(device, s);
    }

    for (int i = 0; i < mDepth.GetMaxDepth(); i++)
    {
        auto s = mDepth.GetDepthSize(i);
        mResiduals.emplace_back(device, s.x*s.y);
        mSmoothers.emplace_back(device, s);
    }

    int depth = mDepth.GetMaxDepth() - 1;
    mSmoother.Bind(mDatas[depth].Diagonal,
                   mDatas[depth].Lower,
                   mDatas[depth].B,
                   mDatas[depth].X);
    mResidualWorkBound.resize(mDepth.GetMaxDepth() + 1);
}

void Multigrid::Bind(Renderer::GenericBuffer& d,
                     Renderer::GenericBuffer& l,
                     Renderer::GenericBuffer& b,
                     Renderer::GenericBuffer& pressure)

{
    mPressure = &pressure;

    mResidualWorkBound[0] =
                mResidualWork.Bind({pressure, d, l, b, mResiduals[0]});
    mSmoothers[0].Bind(d, l, b, pressure);

    auto s = mDepth.GetDepthSize(0);
    mTransfer.RestrictBind(0, s, mResiduals[0], d, mDatas[0].B, mDatas[0].Diagonal);
    mTransfer.ProlongateBind(0, s, pressure, d, mDatas[0].X, mDatas[0].Diagonal);
}

void Multigrid::BuildHierarchiesBind(Pressure& pressure,
                                     Renderer::Texture& solidPhi,
                                     Renderer::Texture& liquidPhi)
{
    auto s = mDepth.GetDepthSize(1);
    mLiquidPhiScaleWorkBound.push_back(mPhiScaleWork.Bind(s, {liquidPhi, mLiquidPhis[0]}));
    mSolidPhiScaleWorkBound.push_back(mPhiScaleWork.Bind(s, {solidPhi, mSolidPhis[0]}));

    RecursiveBind(pressure, 1);

    mBuildHierarchies.Record([&](vk::CommandBuffer commandBuffer)
    {
        commandBuffer.debugMarkerBeginEXT({"Build hierarchies", {{ 0.36f, 0.85f, 0.55f, 1.0f}}});
        for (int i = 0; i < mDepth.GetMaxDepth(); i++)
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

            mMatrixBuildBound[i].PushConstant(commandBuffer, mDelta);
            mMatrixBuildBound[i].Record(commandBuffer);
            mDatas[i].Diagonal.Barrier(commandBuffer, vk::AccessFlagBits::eShaderWrite, vk::AccessFlagBits::eShaderRead);
            mDatas[i].Lower.Barrier(commandBuffer, vk::AccessFlagBits::eShaderWrite, vk::AccessFlagBits::eShaderRead);
            mDatas[i].B.Clear(commandBuffer);
        }

        int maxDepth = mDepth.GetMaxDepth();
        mMatrixBuildBound[maxDepth - 1].PushConstant(commandBuffer, mDelta);
        mMatrixBuildBound[maxDepth - 1].Record(commandBuffer);
        mDatas[maxDepth - 1].Diagonal.Barrier(commandBuffer, vk::AccessFlagBits::eShaderWrite, vk::AccessFlagBits::eShaderRead);
        mDatas[maxDepth - 1].Lower.Barrier(commandBuffer, vk::AccessFlagBits::eShaderWrite, vk::AccessFlagBits::eShaderRead);
        commandBuffer.debugMarkerEndEXT();
    });
}

void Multigrid::RecursiveBind(Pressure& pressure, std::size_t depth)
{
    auto s0 = mDepth.GetDepthSize(depth);

    if (static_cast<int32_t>(depth) < mDepth.GetMaxDepth())
    {
        auto s1 = mDepth.GetDepthSize(depth + 1);
        mLiquidPhiScaleWorkBound.push_back(mPhiScaleWork.Bind(s1, {mLiquidPhis[depth - 1], mLiquidPhis[depth]}));
        mSolidPhiScaleWorkBound.push_back(mPhiScaleWork.Bind(s1, {mSolidPhis[depth - 1], mSolidPhis[depth]}));

        mResidualWorkBound[depth] =
                    mResidualWork.Bind(s0, {mDatas[depth-1].X,
                                           mDatas[depth-1].Diagonal,
                                           mDatas[depth-1].Lower,
                                           mDatas[depth-1].B,
                                           mResiduals[depth]});

        mTransfer.RestrictBind(depth, s0,
                               mResiduals[depth],
                               mDatas[depth-1].Diagonal,
                               mDatas[depth].B,
                               mDatas[depth].Diagonal);

        mTransfer.ProlongateBind(depth, s0,
                                 mDatas[depth-1].X,
                                 mDatas[depth-1].Diagonal,
                                 mDatas[depth].X,
                                 mDatas[depth].Diagonal);

        mSmoothers[depth].Bind(mDatas[depth-1].Diagonal,
            mDatas[depth-1].Lower,
            mDatas[depth-1].B,
            mDatas[depth-1].X);

        RecursiveBind(pressure, depth+1);
    }

    mMatrixBuildBound.push_back(
                pressure.BindMatrixBuild(s0,
                                         mDatas[depth-1].Diagonal,
                                         mDatas[depth-1].Lower,
                                         mLiquidPhis[depth-1],
                                         mSolidPhis[depth-1]));
}

void Multigrid::BuildHierarchies()
{
    mBuildHierarchies.Submit();
}

void Multigrid::Smoother(vk::CommandBuffer commandBuffer, int n, int iterations)
{
  float w = 2.0f / 3.0f;

  mSmoothers[n].SetPreconditionerIterations(iterations);
  mSmoothers[n].SetW(w);
  mSmoothers[n].Record(commandBuffer);
}

void Multigrid::Record(vk::CommandBuffer commandBuffer)
{
    commandBuffer.debugMarkerBeginEXT({"Multigrid", {{ 0.48f, 0.25f, 0.19f, 1.0f}}});

    const int numIterations = 3;

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

    mSmoother.Record(commandBuffer);

    for (int i = mDepth.GetMaxDepth() - 1; i >= 0; --i)
    {
        mTransfer.Prolongate(commandBuffer, i);

        Smoother(commandBuffer, i, numIterations);
    }

    commandBuffer.debugMarkerEndEXT();
}

}}
