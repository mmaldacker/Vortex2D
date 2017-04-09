//
//  GaussSeidel.h
//  Vortex2D
//

#ifndef Vortex2D_GaussSeidel_h
#define Vortex2D_GaussSeidel_h

#include <Vortex2D/Engine/LinearSolver/LinearSolver.h>

namespace Vortex2D { namespace Fluid {

/**
 * @brief An iterative black and red successive over relaxation linear solver.
 */
class GaussSeidel : public LinearSolver
{
public:
    GaussSeidel();
    GaussSeidel(const glm::vec2& size);


    void Build(Data& data,
               Renderer::Operator& diagonals,
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


    /**
     * @brief Will do one step in the red-black gauss seidel solve, given the mask
     */
    void Step(Data& data, uint8_t redMask, uint8_t blackMask);

    void SetW(float w);

private:
    Renderer::Operator mGaussSeidel;
    Renderer::Operator mStencil;
    Renderer::Operator mIdentity;
    Renderer::Uniform<float> mW;
};

}}

#endif
