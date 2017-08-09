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

namespace Vortex2D { namespace Fluid {

class Multigrid : public LinearSolver
{
public:
    Multigrid(const Renderer::Device& device, glm::vec2 size);

    void Build(Renderer::Work& buildMatrix,
               Renderer::Buffer& solidPhi,
               Renderer::Buffer& liquidPhi);

    void Init(Renderer::Buffer& matrix, Renderer::Buffer& b, Renderer::Buffer& pressure) override;

    void Solve(Parameters& params) override;

private:
    void Smoother(Data& data, int iterations);
    void BorderSmoother(Data& data, int iterations, bool up);

    glm::vec2 mSize;
    int mDepths;
    Renderer::Work mResidualWork;
    Renderer::Work mDampedJacobiWork;

    std::vector<Renderer::Work::Bound> mResidualWorkBound;
    std::vector<Renderer::Work::Bound> mDampedJacobiWorkBound;

    Transfer mTransfer;

    std::vector<Renderer::Buffer> mMatrices;
    std::vector<Renderer::Buffer> mPressures;
    std::vector<Renderer::Buffer> mResiduals;
    std::vector<Renderer::Buffer> mSolidPhis;
    std::vector<Renderer::Buffer> mLiquidPhis;

    Renderer::CommandBuffer mCmd;
};

}}

#endif
