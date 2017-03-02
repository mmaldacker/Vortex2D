//
//  Multigrid.h
//  Vortex2D
//

#ifndef Multigrid_h
#define Multigrid_h

#include <Vortex2D/Engine/LinearSolver/LinearSolver.h>
#include <Vortex2D/Renderer/Buffer.h>

namespace Vortex2D { namespace Fluid {

class Multigrid : public LinearSolver
{
public:
    Multigrid(glm::vec2 size);

    void Build(Renderer::Operator& diagonals,
               Renderer::Operator& weights,
               Renderer::Buffer& solidPhi,
               Renderer::Buffer& liquidPhi) override;

    void Init(Data & data) override;

    void Solve(Data& data, Parameters& params) override;

private:
    Data & GetData(int depth);
    void DampedJacobi(Data & data, int iterations = 10);

    int mDepths;
    Renderer::Operator mProlongate;
    Renderer::Operator mResidual;
    Renderer::Operator mRestrict;
    Renderer::Operator mDampedJacobi;
    Renderer::Operator mIdentity;
    Renderer::Operator mMax, mMin;

    std::vector<Data> mDatas;
    std::vector<Renderer::Buffer> mSolidPhis;
    std::vector<Renderer::Buffer> mLiquidPhis;
};

}}

#endif
