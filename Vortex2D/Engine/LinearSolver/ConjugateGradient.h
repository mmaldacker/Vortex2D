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
 * @brief An iterative preconditioned conjugate linear solver. The preconditioner can be specified.
 */
class ConjugateGradient : public LinearSolver
{
public:
    /**
     * @brief Initialize the solver with a size and preconditioner
     * @param device vulkan device
     * @param size
     * @param preconditioner
     */
    VORTEX2D_API ConjugateGradient(const Renderer::Device& device, const glm::ivec2& size, Preconditioner& preconditioner);

    VORTEX2D_API void Bind(Renderer::GenericBuffer& d,
                           Renderer::GenericBuffer& l,
                           Renderer::GenericBuffer& b,
                           Renderer::GenericBuffer& pressure) override;

    VORTEX2D_API void BindRigidbody(float delta,
                                    Renderer::GenericBuffer& d,
                                    RigidBody& rigidBody) override;
    /**
     * @brief Solve iteratively solve the linear equations in data
     */
    VORTEX2D_API void Solve(Parameters& params, const std::vector<RigidBody*>& rigidbodies = {}) override;

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
