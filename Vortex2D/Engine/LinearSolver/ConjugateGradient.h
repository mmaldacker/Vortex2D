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
#include <Vortex2D/Renderer/Timer.h>

namespace Vortex2D { namespace Fluid {

/**
 * @brief An iterative preconditioned conjugate linear solver.
 * The preconditioning operator is a simple diagonal preconditioner.
 */
class ConjugateGradient : public LinearSolver
{
public:
    ConjugateGradient(const Renderer::Device& device, const glm::ivec2& size, Preconditioner& preconditioner);

    void Bind(Renderer::GenericBuffer& d,
              Renderer::GenericBuffer& l,
              Renderer::GenericBuffer& b,
              Renderer::GenericBuffer& pressure) override;

    /**
     * @brief Solve iteratively solve the linear equations in data
     */
    void Solve(Parameters& params) override;

private:
    Preconditioner& mPreconditioner;

    Renderer::Buffer<float> r, s, z, inner, alpha, beta, rho, rho_new, sigma;
    Renderer::Buffer<float> error, localError;
    Renderer::Work matrixMultiply, scalarDivision, scalarMultiply, multiplyAdd, multiplySub;
    ReduceSum reduceSum;
    ReduceMax reduceMax;

    ReduceMax::Bound reduceMaxBound;
    ReduceSum::Bound reduceSumRhoBound, reduceSumSigmaBound, reduceSumRhoNewBound;
    Renderer::Work::Bound multiplySBound, multiplyZBound;
    Renderer::Work::Bound matrixMultiplyBound;
    Renderer::Work::Bound divideRhoBound;
    Renderer::Work::Bound divideRhoNewBound;
    Renderer::Work::Bound multiplyAddPBound, multiplySubRBound, multiplyAddZBound;

    Renderer::CommandBuffer mSolveInit, mSolve;
    Renderer::CommandBuffer mErrorRead;
};

}}

#endif
