//
//  Multigrid.h
//  Vortex2D
//

#ifndef Multigrid_h
#define Multigrid_h

#include <Vortex2D/Engine/LinearSolver/LinearSolver.h>
#include <Vortex2D/Engine/LinearSolver/Preconditioner.h>
#include <Vortex2D/Engine/LinearSolver/Transfer.h>
#include <Vortex2D/Engine/LinearSolver/Jacobi.h>
#include <Vortex2D/Engine/LinearSolver/GaussSeidel.h>
#include <Vortex2D/Engine/LevelSet.h>
#include <Vortex2D/Renderer/Work.h>
#include <Vortex2D/Renderer/CommandBuffer.h>
#include <Vortex2D/Renderer/Texture.h>
#include <Vortex2D/Renderer/Timer.h>

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
    Multigrid(const Renderer::Device& device, const glm::ivec2& size, float delta, bool statistics = false);

    void Init(Renderer::GenericBuffer& d,
              Renderer::GenericBuffer& l,
              Renderer::GenericBuffer& b,
              Renderer::GenericBuffer& x) override;

    void BuildHierarchiesInit(Pressure& pressure,
                              Renderer::Texture& solidPhi,
                              Renderer::Texture& liquidPhi);

    void BuildHierarchies();

    void Record(vk::CommandBuffer commandBuffer) override;

    Renderer::Statistics::Timestamps GetStatistics();

//private:
    void Smoother(vk::CommandBuffer commandBuffer, int n, int iterations);

    void BindRecursive(Pressure& pressure, std::size_t depth);

    Depth mDepth;
    float mDelta;

    Renderer::Work mResidualWork;
    std::vector<Renderer::Work::Bound> mResidualWorkBound;

    Transfer mTransfer;

    Renderer::GenericBuffer* mPressure = nullptr;

    // mDatas[0]  is level 1
    std::vector<LinearSolver::Data> mDatas;

    // mResiduals[0] is level 0
    std::vector<Renderer::Buffer<float>> mResiduals;

    Renderer::Work mPhiScaleWork;
    std::vector<Renderer::Work::Bound> mSolidPhiScaleWorkBound;
    std::vector<Renderer::Work::Bound> mLiquidPhiScaleWorkBound;

    // mSolidPhis[0] and mLiquidPhis[0] is level 1
    std::vector<LevelSet> mSolidPhis;
    std::vector<LevelSet> mLiquidPhis;

    std::vector<Renderer::Work::Bound> mMatrixBuildBound;

    // mSmoothers[0] is level 0
    std::vector<Jacobi> mSmoothers;
    LocalGaussSeidel mSmoother;

    Renderer::CommandBuffer mBuildHierarchies;

    bool mEnableStatistics;
    Renderer::Statistics mStatistics;
};

}}

#endif
