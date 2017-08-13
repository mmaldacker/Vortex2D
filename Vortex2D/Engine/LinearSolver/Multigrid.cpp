//
//  Multigrid.cpp
//  Vortex2D
//

#include "Multigrid.h"

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
    , mBoundaryGaussSeidelWork(device, size, "../Vortex2D/BoundaryGaussSeidel.comp.spv",
                              {vk::DescriptorType::eStorageImage,
                               vk::DescriptorType::eStorageBuffer,
                               vk::DescriptorType::eStorageBuffer,
                               vk::DescriptorType::eStorageBuffer},
                               8)
    , mTransfer(device)
    , mPressureBack(device, vk::BufferUsageFlagBits::eStorageBuffer, false, size.x*size.y*sizeof(float))
    , mPhiScaleWork(device, size, "../Vortex2D/PhiScale.comp.spv",
                    {vk::DescriptorType::eStorageImage,
                     vk::DescriptorType::eStorageImage})
    , mCmd(device, false)
{
    for (int i = 1; i <= mDepth.GetMaxDepth(); i++)
    {
        auto s = mDepth.GetDepthSize(i);

        mMatrices.emplace_back(device, vk::BufferUsageFlagBits::eStorageBuffer, false, s.x*s.y*sizeof(Data));
        mPressures.emplace_back(device, vk::BufferUsageFlagBits::eStorageBuffer, false, s.x*s.y*sizeof(float));
        mPressuresBack.emplace_back(device, vk::BufferUsageFlagBits::eStorageBuffer, false, s.x*s.y*sizeof(float));
        mBs.emplace_back(device, vk::BufferUsageFlagBits::eStorageBuffer, false, s.x*s.y*sizeof(float));

        mSolidPhis.emplace_back(device, s.x, s.y, vk::Format::eR32Sfloat, false);
        mLiquidPhis.emplace_back(device, s.x, s.y, vk::Format::eR32Sfloat, false);

        ExecuteCommand(device, [&](vk::CommandBuffer commandBuffer)
        {
            mSolidPhis.back().Clear(commandBuffer, {{-1.0f, 0.0f, 0.0f, 0.0f}});
            mLiquidPhis.back().Clear(commandBuffer, {{1.0f, 0.0f, 0.0f, 0.0f}});
        });
    }

    for (int i = 0; i < mDepth.GetMaxDepth(); i++)
    {
        auto s = mDepth.GetDepthSize(i);
        mResiduals.emplace_back(device, vk::BufferUsageFlagBits::eStorageBuffer, false, s.x*s.y*sizeof(float));
    }
}

void Multigrid::Init(Renderer::Buffer& matrix,
                     Renderer::Buffer& b,
                     Renderer::Buffer& pressure,
                     Renderer::Work& buildMatrix,
                     Renderer::Texture& solidPhi,
                     Renderer::Texture& liquidPhi)
{

    auto s = mDepth.GetDepthSize(1);
    mLiquidPhiScaleWorkBound.push_back(mPhiScaleWork.Bind(s, {liquidPhi, mLiquidPhis[0]}));
    mSolidPhiScaleWorkBound.push_back(mPhiScaleWork.Bind(s, {solidPhi, mSolidPhis[0]}));

    for (int i = 1; i < mDepth.GetMaxDepth(); i++)
    {
        auto s1 = mDepth.GetDepthSize(i + 1);
        mLiquidPhiScaleWorkBound.push_back(mPhiScaleWork.Bind(s1, {mLiquidPhis[i-1], mLiquidPhis[i]}));
        mSolidPhiScaleWorkBound.push_back(mPhiScaleWork.Bind(s1, {mSolidPhis[i-1], mSolidPhis[i]}));

        auto s0 = mDepth.GetDepthSize(i);
        mMatrixBuildBound.push_back(buildMatrix.Bind(s0, {mMatrices[i-1], mLiquidPhis[i-1], mSolidPhis[i-1]}));
    }

    int maxDepth = mDepth.GetMaxDepth();
    mMatrixBuildBound.push_back(buildMatrix.Bind(mDepth.GetDepthSize(maxDepth), {mMatrices[maxDepth-1], mLiquidPhis[maxDepth-1], mSolidPhis[maxDepth-1]}));

    mPressure = &pressure;
    mResidualWorkBound.push_back(
                mResidualWork.Bind({pressure, matrix, b, mResiduals[0]}));

    mTransfer.InitRestrict(mDepth.GetDepthSize(0), mResiduals[0], mBs[0]);
    mTransfer.InitProlongate(mDepth.GetDepthSize(0), pressure, mPressures[0], matrix);

    mDampedJacobiWorkBound.emplace_back(
                mDampedJacobiWork.Bind({pressure, matrix, b, mPressureBack}),
                mDampedJacobiWork.Bind({mPressureBack, matrix, b, pressure}));

    mBoundaryGaussSeidelBound.emplace_back(
                mBoundaryGaussSeidelWork.Bind({liquidPhi, pressure, matrix, b}));

    for (int i = 1; i < mDepth.GetMaxDepth(); i++)
    {
        auto s = mDepth.GetDepthSize(i);
        mResidualWorkBound.push_back(
                    mResidualWork.Bind(s, {mPressures[i-1], mMatrices[i-1], mBs[i-1], mResiduals[i]}));

        mTransfer.InitRestrict(s, mResiduals[i], mBs[i]);
        mTransfer.InitProlongate(s, mPressures[i-1], mPressures[i], mMatrices[i-1]);

        mDampedJacobiWorkBound.emplace_back(
                    mDampedJacobiWork.Bind(s, {mPressures[i-1], mMatrices[i-1], mBs[i-1], mPressuresBack[i-1]}),
                    mDampedJacobiWork.Bind(s, {mPressuresBack[i-1], mMatrices[i-1], mBs[i-1], mPressures[i-1]}));
        mBoundaryGaussSeidelBound.emplace_back(
                    mBoundaryGaussSeidelWork.Bind(s, {mLiquidPhis[i-1], mPressures[i-1], mMatrices[i-1], mBs[i-1]}));
    }

    mDampedJacobiWorkBound.emplace_back(
                mDampedJacobiWork.Bind(mDepth.GetDepthSize(maxDepth), {mPressures[maxDepth-1], mMatrices[maxDepth-1], mBs[maxDepth-1], mPressuresBack[maxDepth-1]}),
                mDampedJacobiWork.Bind(mDepth.GetDepthSize(maxDepth), {mPressuresBack[maxDepth-1], mMatrices[maxDepth-1], mBs[maxDepth-1], mPressures[maxDepth-1]}));
    mBoundaryGaussSeidelBound.emplace_back(
                mBoundaryGaussSeidelWork.Bind(mDepth.GetDepthSize(maxDepth), {mLiquidPhis[maxDepth-1], mPressures[maxDepth-1], mMatrices[maxDepth-1], mBs[maxDepth-1]}));

    mCmd.Record([&](vk::CommandBuffer commandBuffer)
    {
        RecordInit(commandBuffer);
        Record(commandBuffer);
    });
}

void Multigrid::Smoother(vk::CommandBuffer commandBuffer, int n, int iterations)
{
    for (int i = 0; i < iterations; i++)
    {
        if (n == 0)
        {
            mDampedJacobiWorkBound[n].first.Record(commandBuffer);
            mPressureBack.Barrier(commandBuffer, vk::AccessFlagBits::eShaderWrite, vk::AccessFlagBits::eShaderRead);
            mDampedJacobiWorkBound[n].second.Record(commandBuffer);
            assert(mPressure);
            mPressure->Barrier(commandBuffer, vk::AccessFlagBits::eShaderWrite, vk::AccessFlagBits::eShaderRead);
        }
        else
        {
            mDampedJacobiWorkBound[n].first.Record(commandBuffer);
            mPressuresBack[n-1].Barrier(commandBuffer, vk::AccessFlagBits::eShaderWrite, vk::AccessFlagBits::eShaderRead);
            mDampedJacobiWorkBound[n].second.Record(commandBuffer);
            mPressures[n-1].Barrier(commandBuffer, vk::AccessFlagBits::eShaderWrite, vk::AccessFlagBits::eShaderRead);
        }
    }
}

void Multigrid::BorderSmoother(vk::CommandBuffer commandBuffer, int n, int iterations, bool up)
{
    for (int i = 0; i < iterations; i++)
    {
        if (n == 0)
        {
            mBoundaryGaussSeidelBound[n].PushConstant(commandBuffer, 8, 1.0f);
            mBoundaryGaussSeidelBound[n].PushConstant(commandBuffer, 12, up ? 1 : 0);
            mBoundaryGaussSeidelBound[n].Record(commandBuffer);
            assert(mPressure);
            mPressure->Barrier(commandBuffer, vk::AccessFlagBits::eShaderWrite, vk::AccessFlagBits::eShaderRead);
            mBoundaryGaussSeidelBound[n].PushConstant(commandBuffer, 12, up ? 0 : 1);
            mBoundaryGaussSeidelBound[n].Record(commandBuffer);
            mPressure->Barrier(commandBuffer, vk::AccessFlagBits::eShaderWrite, vk::AccessFlagBits::eShaderRead);
        }
        else
        {
            mBoundaryGaussSeidelBound[n].PushConstant(commandBuffer, 8, 1.0f);
            mBoundaryGaussSeidelBound[n].PushConstant(commandBuffer, 12, up ? 1 : 0);
            mBoundaryGaussSeidelBound[n].Record(commandBuffer);
            mPressuresBack[n-1].Barrier(commandBuffer, vk::AccessFlagBits::eShaderWrite, vk::AccessFlagBits::eShaderRead);
            mBoundaryGaussSeidelBound[n].PushConstant(commandBuffer, 12, up ? 0 : 1);
            mBoundaryGaussSeidelBound[n].Record(commandBuffer);
            mPressuresBack[n-1].Barrier(commandBuffer, vk::AccessFlagBits::eShaderWrite, vk::AccessFlagBits::eShaderRead);
        }
    }
}

void Multigrid::Solve(Parameters&)
{
    mCmd.Submit();
}

void Multigrid::RecordInit(vk::CommandBuffer commandBuffer)
{
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

        mMatrixBuildBound[i].PushConstant(commandBuffer, 8, mDelta);
        mMatrixBuildBound[i].Record(commandBuffer);
        mMatrices[i].Barrier(commandBuffer, vk::AccessFlagBits::eShaderWrite, vk::AccessFlagBits::eShaderRead);
    }

    int maxDepth = mDepth.GetMaxDepth();
    mMatrixBuildBound[maxDepth - 1].PushConstant(commandBuffer, 8, mDelta);
    mMatrixBuildBound[maxDepth - 1].Record(commandBuffer);
    mMatrices[maxDepth - 1].Barrier(commandBuffer, vk::AccessFlagBits::eShaderWrite, vk::AccessFlagBits::eShaderRead);

}

void Multigrid::Record(vk::CommandBuffer commandBuffer)
{
    int numIterations = 2;
    int numBorderIterations = 2;
    int numIterationsCoarse = 32;

    assert(mPressure != nullptr);
    mPressure->Clear(commandBuffer);

    for (int i = 0; i < mDepth.GetMaxDepth(); i++)
    {
        Smoother(commandBuffer, i, numIterations);
        BorderSmoother(commandBuffer, i, numBorderIterations, true);

        numIterations *= 2;

        mResidualWorkBound[i].Record(commandBuffer);
        mResiduals[i].Barrier(commandBuffer, vk::AccessFlagBits::eShaderWrite, vk::AccessFlagBits::eShaderRead);
        mTransfer.Restrict(commandBuffer, i);
        mPressures[i].Clear(commandBuffer);
    }

    Smoother(commandBuffer, mDepth.GetMaxDepth(), numIterationsCoarse);

    for (int i = mDepth.GetMaxDepth() - 1; i >= 0; --i)
    {
        mTransfer.Prolongate(commandBuffer, i);

        numIterations /= 2;

        BorderSmoother(commandBuffer, i, numBorderIterations, false);
        Smoother(commandBuffer, i, numIterations);
    }
}


}}
