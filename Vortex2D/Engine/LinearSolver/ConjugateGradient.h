//
//  ConjugateGradient.h
//  Vertex2D
//

#ifndef Vertex2D_ConjugateGradient_h
#define Vertex2D_ConjugateGradient_h

#include <Vortex2D/Renderer/Reader.h>

#include <Vortex2D/Engine/LinearSolver/LinearSolver.h>
#include <Vortex2D/Engine/LinearSolver/Multigrid.h>
#include <Vortex2D/Engine/LinearSolver/Reduce.h>

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

    void Build(Data& data,
               Renderer::Operator& diagonals,
               Renderer::Operator& weights,
               Renderer::Buffer& solidPhi,
               Renderer::Buffer& liquidPhi) override;

    /**
     * @brief Empty implementation as there are no initialisation for CG
     */
    void Init(Data& data) override;

    /**
     * @brief Solve iteratively solve the linear equations in data
     */
    void Solve(Data& data, Parameters& params) override;

    void NormalSolve(Data& data, Parameters& params);

private:
    void ApplyPreconditioner(Data& data);
    void InnerProduct(Renderer::Buffer& output, Renderer::Buffer& intput1, Renderer::Buffer& input2);

    Renderer::Buffer r, s, alpha, beta, rho, rho_new, sigma, error;
    Renderer::Operator matrixMultiply, scalarDivision, multiplyAdd, multiplySub, residual, identity;
    ReduceSum reduceSum;
    ReduceMax reduceMax;
    Renderer::Reader errorReader;

    Renderer::Operator swizzle;
    Data z;
    Multigrid preconditioner;
};

}}

#endif
