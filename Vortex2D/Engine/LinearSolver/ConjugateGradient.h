//
//  ConjugateGradient.h
//  Vertex2D
//

#ifndef Vertex2D_ConjugateGradient_h
#define Vertex2D_ConjugateGradient_h

#include <Vortex2D/Engine/LinearSolver/LinearSolver.h>
#include <Vortex2D/Engine/LinearSolver/Reduce.h>
#include <Vortex2D/Renderer/Work.h>

namespace Vortex2D { namespace Fluid {

/**
 * @brief An iterative preconditioned conjugate linear solver.
 * The preconditioning operator is a simple diagonal preconditioner.
 */
class ConjugateGradient : public LinearSolver
{
public:
    ConjugateGradient(const glm::vec2& size);
    virtual ~ConjugateGradient();


    void Init(Renderer::Buffer& data, Renderer::Buffer& pressure) override;

    /**
     * @brief Solve iteratively solve the linear equations in data
     */
    void Solve(Parameters& params) override;

    void NormalSolve(Parameters& params);

private:
    void ApplyPreconditioner(Data& data);
    void InnerProduct(Renderer::Buffer& output, Renderer::Buffer& intput1, Renderer::Buffer& input2);

    Renderer::Buffer r, s, alpha, beta, rho, rho_new, sigma, error;
    Renderer::Work matrixMultiply, scalarDivision, multiplyAdd, multiplySub, residual, identity;
    ReduceSum reduceSum;
    ReduceMax reduceMax;
};

}}

#endif
