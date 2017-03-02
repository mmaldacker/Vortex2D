//
//  SuccessiveOverRelaxation.h
//  Vortex2D
//

#ifndef Vortex2D_SuccessiveOverRelaxation_h
#define Vortex2D_SuccessiveOverRelaxation_h

#include <Vortex2D/Engine/LinearSolver/LinearSolver.h>

namespace Vortex2D { namespace Fluid {

/**
 * @brief An iterative black and red successive over relaxation linear solver.
 */
class SuccessiveOverRelaxation : public LinearSolver
{
public:
    SuccessiveOverRelaxation(const glm::vec2& size);
    SuccessiveOverRelaxation(const glm::vec2& size, float w);


    void Build(Renderer::Operator& diagonals,
               Renderer::Operator& weights,
               Renderer::Buffer& solidPhi,
               Renderer::Buffer& liquidPhi) override;

    /**
     * @brief Sets up the stencil mask to enable red-black solving
     */
    void Init(Data& data) override;

    /**
     * @brief Iterative solving of the linear equations in data
     */
    void Solve(Data& data, Parameters& params) override;

private:
    void Step(Data& data, bool isRed);

    Renderer::Operator mSor;
    Renderer::Operator mStencil;
    Renderer::Operator mIdentity;
};

}}

#endif
