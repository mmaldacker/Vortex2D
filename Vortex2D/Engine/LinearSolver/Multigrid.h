//
//  Multigrid.h
//  Vortex2D
//

#ifndef Multigrid_h
#define Multigrid_h

#include <Vortex2D/Engine/LinearSolver/LinearSolver.h>
#include <Vortex2D/Engine/LinearSolver/Transfer.h>
#include <Vortex2D/Engine/LinearSolver/GaussSeidel.h>
#include <Vortex2D/Renderer/Data.h>

namespace Vortex2D { namespace Fluid {

class Multigrid : public LinearSolver
{
public:
    Multigrid(glm::vec2 size);

    void Build(Data& data,
               Renderer::Operator& diagonals,
               Renderer::Operator& weights,
               Renderer::Buffer& solidPhi,
               Renderer::Buffer& liquidPhi) override;

    void Init(Data& data) override;

    void Solve(Data& data, Parameters& params) override;

private:
    void Smoother(Data& data, int iterations);
    void BorderSmoother(Data& data, int iterations, bool up);

    void RenderBoundaryMask(Data& data, Renderer::Buffer& buffer);

    int mDepths;
    Renderer::Operator mResidual;
    Renderer::Operator mDampedJacobi;
    Renderer::Operator mIdentity;
    Renderer::Operator mScale;
    Renderer::Operator mBoundaryMask;

    Transfer mTransfer;
    GaussSeidel mGaussSeidel;

    std::vector<Data> mDatas;
    std::vector<Renderer::Buffer> mSolidPhis;
    std::vector<Renderer::Buffer> mLiquidPhis;
};

}}

#endif
