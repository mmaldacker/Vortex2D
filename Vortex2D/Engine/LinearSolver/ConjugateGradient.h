//
//  ConjugateGradient.h
//  Vertex2D
//

#ifndef Vertex2D_ConjugateGradient_h
#define Vertex2D_ConjugateGradient_h

#include <Vortex2D/Engine/LinearSolver/LinearSolver.h>
#include <Vortex2D/Engine/LinearSolver/Preconditioner.h>
#include <Vortex2D/Engine/LinearSolver/Reduce.h>
#include <Vortex2D/Renderer/Work.h>
#include <Vortex2D/Renderer/CommandBuffer.h>

namespace Vortex2D { namespace Fluid {

/**
 * @brief An iterative preconditioned conjugate linear solver.
 * The preconditioning operator is a simple diagonal preconditioner.
 */
class ConjugateGradient : public LinearSolver
{
public:
    ConjugateGradient(const Renderer::Device& device, const glm::ivec2& size, Preconditioner& preconditioner);

    void Init(Renderer::Buffer& A,
              Renderer::Buffer& b,
              Renderer::Buffer& pressure,
              Renderer::Work& buildMatrix,
              Renderer::Texture& solidPhi,
              Renderer::Texture& liquidPhi) override;

    /**
     * @brief Solve iteratively solve the linear equations in data
     */
    void Solve(Parameters& params) override;

    void NormalSolve(Parameters& params);

//private:
    Preconditioner& mPreconditioner;

    Renderer::Buffer r, s, alpha, beta, rho, rho_new, sigma, error, errorLocal, inner, z;
    Renderer::Work matrixMultiply, scalarDivision, scalarMultiply, multiplyAdd, multiplySub;
    ReduceSum reduceSum;
    ReduceMax reduceMax;

    ReduceMax::Bound reduceMaxBound;
    ReduceSum::Bound reduceSumRhoBound, reduceSumSigmaBound, reduceSumRhoNewBound;
    Renderer::Work::Bound multiplyRBound, multiplySBound, multiplyZBound;
    Renderer::Work::Bound matrixMultiplyBound;
    Renderer::Work::Bound divideRhoBound;
    Renderer::Work::Bound divideRhoNewBound;
    Renderer::Work::Bound multiplyAddPBound, multiplySubRBound, multiplyAddSBound, multiplyAddZBound;

    Renderer::CommandBuffer mNormalSolveInit, mNormalSolve;
    Renderer::CommandBuffer mSolveInit, mSolve;
    Renderer::CommandBuffer mErrorRead;
};

}}

#endif
