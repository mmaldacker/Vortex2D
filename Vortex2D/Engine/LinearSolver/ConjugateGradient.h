//
//  ConjugateGradient.h
//  Vertex2D
//

#ifndef Vertex2D_ConjugateGradient_h
#define Vertex2D_ConjugateGradient_h

#include "LinearSolver.h"
#include "Reduce.h"

namespace Vortex2D { namespace Fluid {

/**
 * @brief An iterative preconditioned conjugate linear solver.
 * The preconditioning operator is a simple diagonal preconditioner.
 */
class ConjugateGradient : public LinearSolver
{
public:
    ConjugateGradient(const glm::vec2& size, unsigned iterations = 40);
    virtual ~ConjugateGradient();

    /**
     * @brief Empty implementation as there are no initialisation for CG
     */
    void Init(LinearSolver::Data& data) override;

    /**
     * @brief Solve iteratively solve the linear equations in data
     */
    void Solve(LinearSolver::Data& data) override;

    void NormalSolve(LinearSolver::Data& data);

private:
    void ApplyPreconditioner(LinearSolver::Data& data);

    Renderer::Buffer r, s, z, alpha, beta, rho, rho_new, sigma;
    Renderer::Operator matrixMultiply, scalarDivision, multiplyAdd, multiplySub, residual, identity;
    Reduce reduce;
    unsigned mIterations;
};

}}

#endif
