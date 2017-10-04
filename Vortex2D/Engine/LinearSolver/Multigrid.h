//
//  Multigrid.h
//  Vortex2D
//

#ifndef Multigrid_h
#define Multigrid_h

#include <Vortex2D/Engine/LinearSolver/LinearSolver.h>
#include <Vortex2D/Engine/LinearSolver/Preconditioner.h>
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

class Pressure;

class Multigrid : public Preconditioner
{
public:
    Multigrid(const Renderer::Device& device, const glm::ivec2& size, float delta);

    void Init(Renderer::Buffer& d,
              Renderer::Buffer& l,
              Renderer::Buffer& b,
              Renderer::Buffer& x) override;

    void Build(Pressure& pressure,
               Renderer::Texture& solidPhi,
               Renderer::Texture& liquidPhi);

    void RecordInit(vk::CommandBuffer commandBuffer) override;
    void Record(vk::CommandBuffer commandBuffer) override;

//private:
    void Smoother(vk::CommandBuffer commandBuffer, int n, int iterations);

    void BuildRecursive(Pressure& pressure, std::size_t depth);

    Depth mDepth;
    float mDelta;

    Renderer::Work mResidualWork;
    std::vector<Renderer::Work::Bound> mResidualWorkBound;

    Transfer mTransfer;

    Renderer::Buffer* mPressure = nullptr;

    // mDatas[0]  is level 1
    std::vector<LinearSolver::Data> mDatas;

    // mResiduals[0] is level 0
    std::vector<Renderer::Buffer> mResiduals;

    Renderer::Work mPhiScaleWork;
    std::vector<Renderer::Work::Bound> mSolidPhiScaleWorkBound;
    std::vector<Renderer::Work::Bound> mLiquidPhiScaleWorkBound;

    // mSolidPhis[0] and mLiquidPhis[0] is level 1
    std::vector<Renderer::Texture> mSolidPhis;
    std::vector<Renderer::Texture> mLiquidPhis;

    std::vector<Renderer::Work::Bound> mMatrixBuildBound;

    std::vector<GaussSeidel> mSmoothers;
};

}}

#endif
