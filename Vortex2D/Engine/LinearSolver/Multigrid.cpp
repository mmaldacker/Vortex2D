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

Multigrid::Multigrid(const Renderer::Device& device, const glm::ivec2& size)
  : mDepth(size)
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
  , mPressureBack(device, vk::BufferUsageFlagBits::eStorageBuffer, false, size.x*size.y*sizeof(float))
  , mCoarseMinWork(device, size, "../Vortex2D/CoarseMin.comp.spv",
    {vk::DescriptorType::eStorageImage,
    vk::DescriptorType::eStorageImage})
  , mCoarseMaxWork(device, size, "../Vortex2D/CoarseMax.comp.spv",
    {vk::DescriptorType::eStorageImage,
    vk::DescriptorType::eStorageImage})
  , mBuildCmd(device, false)
  , mCmd(device, false)
{
  for (int i = 1; i <= mDepth.GetMaxDepth(); i++)
  {
    mMatrices.emplace_back(device, vk::BufferUsageFlagBits::eStorageBuffer, false, size.x*size.y*sizeof(Data));
    mPressures.emplace_back(device, vk::BufferUsageFlagBits::eStorageBuffer, false, size.x*size.y*sizeof(float));
    mPressuresBack.emplace_back(device, vk::BufferUsageFlagBits::eStorageBuffer, false, size.x*size.y*sizeof(float));
    mBs.emplace_back(device, vk::BufferUsageFlagBits::eStorageBuffer, false, size.x*size.y*sizeof(float));

    mSolidPhis.emplace_back(device, size.x, size.y, vk::Format::eR32Sfloat, false);
    mLiquidPhis.emplace_back(device, size.x, size.y, vk::Format::eR32Sfloat, false);
  }

  for (int i = 0; i < mDepth.GetMaxDepth(); i++)
  {
    mResiduals.emplace_back(device, vk::BufferUsageFlagBits::eStorageBuffer, false, size.x*size.y*sizeof(float));
  }
}

void Multigrid::Init(Renderer::Buffer& matrix, Renderer::Buffer& b, Renderer::Buffer& pressure)
{
  mPressure = &pressure;

  mResidualWorkBound.push_back(
        mResidualWork.Bind(mDepth.GetDepthSize(0), {pressure, matrix, b, mResiduals[0]}));
  mDampedJacobiWorkBound.emplace_back(
        mDampedJacobiWork.Bind(mDepth.GetDepthSize(0), {pressure, matrix, b, mPressureBack}),
        mDampedJacobiWork.Bind(mDepth.GetDepthSize(0), {mPressureBack, matrix, b, pressure}));
  mTransfer.Init(mDepth.GetDepthSize(0), mResiduals[0], mBs[0]);

  for (int i = 1; i < mDepth.GetMaxDepth(); i++)
  {
    mResidualWorkBound.push_back(
          mResidualWork.Bind(mDepth.GetDepthSize(i), {mPressures[i-1], mMatrices[i-1], mBs[i-1], mResiduals[i]}));
    mTransfer.Init(mDepth.GetDepthSize(i), mResiduals[i], mBs[i]);

  }

  for (int i = 1; i <= mDepth.GetMaxDepth(); i++)
  {
    mDampedJacobiWorkBound.emplace_back(
          mDampedJacobiWork.Bind(mDepth.GetDepthSize(i), {mPressures[i-1], mMatrices[i-1], mBs[i-1], mPressuresBack[i-1]}),
          mDampedJacobiWork.Bind(mDepth.GetDepthSize(i), {mPressuresBack[i-1], mMatrices[i-1], mBs[i-1], mPressures[i-1]}));
  }

  mCmd.Record([&](vk::CommandBuffer commandBuffer)
  {
    int numIterations = 4;
    int numBorderIterations = 2;
    int numIterationsCoarse = 32;

    pressure.Clear(commandBuffer);

    for (int i = 0; i < mDepth.GetMaxDepth(); i++)
    {
      numIterations *= 2;

      Smoother(commandBuffer, i, numIterations);
      BorderSmoother(commandBuffer, i, numBorderIterations, true);

      mResidualWorkBound[i].Record(commandBuffer);
      mResiduals[i].Barrier(commandBuffer, vk::AccessFlagBits::eShaderWrite, vk::AccessFlagBits::eShaderRead);
      mTransfer.Restrict(i);
      mPressures[i].Clear(commandBuffer);
    }

    Smoother(commandBuffer, mDepth.GetMaxDepth(), numIterationsCoarse);

    for (int i = mDepth.GetMaxDepth() - 1; i >= 0; --i)
    {
      mTransfer.Prolongate(i);

      BorderSmoother(commandBuffer, i, numBorderIterations, false);
      Smoother(commandBuffer, i, numIterations);

      numIterations /= 2;
    }
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
      mDampedJacobiWorkBound[n].first.Record(commandBuffer);
      assert(mPressure);
      mPressure->Barrier(commandBuffer, vk::AccessFlagBits::eShaderWrite, vk::AccessFlagBits::eShaderRead);
    }
    else
    {
      mDampedJacobiWorkBound[n].first.Record(commandBuffer);
      mPressuresBack[n].Barrier(commandBuffer, vk::AccessFlagBits::eShaderWrite, vk::AccessFlagBits::eShaderRead);
      mDampedJacobiWorkBound[n].first.Record(commandBuffer);
      mPressures[n].Barrier(commandBuffer, vk::AccessFlagBits::eShaderWrite, vk::AccessFlagBits::eShaderRead);
    }
  }
}

void Multigrid::BorderSmoother(vk::CommandBuffer commandBuffer, int n, int iterations, bool up)
{
  for (int i = 0; i < iterations; i++)
  {

  }
}

void Multigrid::Build(Renderer::Work& buildMatrix,
                      Renderer::Texture& solidPhi,
                      Renderer::Texture& liquidPhi)
{

  mCoarseMinWorkBound.push_back(mCoarseMinWork.Bind({liquidPhi, mLiquidPhis[0]}));
  mCoarseMaxWorkBound.push_back(mCoarseMaxWork.Bind({solidPhi, mSolidPhis[0]}));

  for (int i = 1; i < mDepth.GetMaxDepth(); i++)
  {
    mCoarseMinWorkBound.push_back(mCoarseMinWork.Bind({mLiquidPhis[i-1], mLiquidPhis[i]}));
    mCoarseMaxWorkBound.push_back(mCoarseMaxWork.Bind({mSolidPhis[i-1], mSolidPhis[i]}));
    mMatrixBuildBound.push_back(buildMatrix.Bind({mMatrices[i-1], mLiquidPhis[i-1], mSolidPhis[i-1]}));
  }

  mBuildCmd.Record([&](vk::CommandBuffer commandBuffer)
  {
    for (int i = 0; i < mDepth.GetMaxDepth(); i++)
    {
      mCoarseMinWorkBound[i].Record(commandBuffer);
      mLiquidPhis[i+1].Barrier(commandBuffer,
                               vk::ImageLayout::eGeneral,
                               vk::AccessFlagBits::eShaderWrite,
                               vk::ImageLayout::eGeneral,
                               vk::AccessFlagBits::eShaderRead);

      mCoarseMaxWorkBound[i].Record(commandBuffer);
      mSolidPhis[i+1].Barrier(commandBuffer,
                              vk::ImageLayout::eGeneral,
                              vk::AccessFlagBits::eShaderWrite,
                              vk::ImageLayout::eGeneral,
                              vk::AccessFlagBits::eShaderRead);

      // TODO need to set delta?
      mMatrixBuildBound[i].Record(commandBuffer);
      mMatrices[i].Barrier(commandBuffer, vk::AccessFlagBits::eShaderWrite, vk::AccessFlagBits::eShaderRead);
    }
  });
}

void Multigrid::Solve(Parameters&)
{
  mBuildCmd.Submit();
  mCmd.Submit();
}

}}
