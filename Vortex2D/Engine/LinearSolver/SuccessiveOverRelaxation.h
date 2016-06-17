//
//  SuccessiveOverRelaxation.h
//  Vertex2D
//

#ifndef __Vertex2D__SuccessiveOverRelaxation__
#define __Vertex2D__SuccessiveOverRelaxation__

#include "LinearSolver.h"

namespace Vortex2D { namespace Fluid {

/**
 * @brief An iterative black and red successive over relaxation linear solver.
 */
class SuccessiveOverRelaxation : public LinearSolver
{
public:
    SuccessiveOverRelaxation(const glm::vec2 & size, int iterations = 40);
    SuccessiveOverRelaxation(const glm::vec2 & size, int iterations, float w);

    /**
     * @brief Sets up the stencil mask to enable red-black solving
     */
    void Init(LinearSolver::Data & data) override;

    /**
     * @brief Iterative solving of the linear equations in data
     */
    void Solve(LinearSolver::Data & data) override;

private:
    void Step(LinearSolver::Data & data, bool isRed);

    int mIterations;

    Operator mSor;
    Operator mStencil;
    Operator mIdentity;
};

}}

#endif /* defined(__Vertex2D__SuccessiveOverRelaxation__) */
