//
//  Multigrid.h
//  Vortex2D
//

#ifndef Multigrid_h
#define Multigrid_h

#include <Vortex2D/Engine/LinearSolver/LinearSolver.h>
#include <Vortex2D/Engine/LinearSolver/Transfer.h>
#include <Vortex2D/Engine/LinearSolver/GaussSeidel.h>
#include <Vortex2D/Renderer/Work.h>
#include <Vortex2D/Renderer/CommandBuffer.h>
#include <Vortex2D/Renderer/Texture.h>

namespace Vortex2D { namespace Fluid {

class Depth
{
public:
  Depth(const glm::ivec2& size);

  int GetMaxDepth() const;
  glm::ivec2 GetDepthSize(int i) const;

private:
  std::vector<glm::ivec2> mDepths;
};

class Multigrid : public LinearSolver
{
public:
    Multigrid(const Renderer::Device& device, const glm::ivec2& size, float delta);

    void Init(Renderer::Buffer& matrix,
              Renderer::Buffer& b,
              Renderer::Buffer& pressure,
              Renderer::Work& buildMatrix,
              Renderer::Texture& solidPhi,
              Renderer::Texture& liquidPhi) override;

    void Solve(Parameters& params) override;

//private:
    void Smoother(vk::CommandBuffer commandBuffer, int n, int iterations);
    void BorderSmoother(vk::CommandBuffer commandBuffer, int n, int iterations, bool up);

    Depth mDepth;
    float mDelta;
    Renderer::Work mResidualWork;
    Renderer::Work mDampedJacobiWork;

    std::vector<Renderer::Work::Bound> mResidualWorkBound;
    std::vector<std::pair<Renderer::Work::Bound, Renderer::Work::Bound>> mDampedJacobiWorkBound;

    Transfer mTransfer;

    Renderer::Buffer* mPressure = nullptr;
    Renderer::Buffer mPressureBack;

    // mMatrices[0] is level 1
    std::vector<Renderer::Buffer> mMatrices;
    // mPressures[0] and mPressuresBack[0] is level 1
    std::vector<Renderer::Buffer> mPressures;
    std::vector<Renderer::Buffer> mPressuresBack;
    // mBs[0] is level 1
    std::vector<Renderer::Buffer> mBs;
    // mResiduals[0] is level 0
    std::vector<Renderer::Buffer> mResiduals;

    Renderer::Work mPhiScaleWork;
    std::vector<Renderer::Work::Bound> mSolidPhiScaleWorkBound;
    std::vector<Renderer::Work::Bound> mLiquidPhiScaleWorkBound;

    // mSolidPhis[0] and mLiquidPhis[0] is level 1
    std::vector<Renderer::Texture> mSolidPhis;
    std::vector<Renderer::Texture> mLiquidPhis;

    std::vector<Renderer::Work::Bound> mMatrixBuildBound;

    Renderer::CommandBuffer mBuildCmd;
    Renderer::CommandBuffer mCmd;
};

}}

#endif
