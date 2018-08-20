//
//  GaussSeidel.h
//  Vortex2D
//

#ifndef Vortex2D_GaussSeidel_h
#define Vortex2D_GaussSeidel_h

#include <Vortex2D/Engine/LinearSolver/LinearSolver.h>
#include <Vortex2D/Engine/LinearSolver/Preconditioner.h>
#include <Vortex2D/Engine/LinearSolver/Reduce.h>
#include <Vortex2D/Renderer/Work.h>
#include <Vortex2D/Renderer/CommandBuffer.h>
#include <Vortex2D/Renderer/Pipeline.h>

namespace Vortex2D { namespace Fluid {

/**
 * @brief An iterative black and red successive over relaxation linear solver.
 */
class GaussSeidel : public LinearSolver, public Preconditioner
{
public:
    VORTEX2D_API GaussSeidel(const Renderer::Device& device, const glm::ivec2& size);

    VORTEX2D_API void Bind(Renderer::GenericBuffer& d,
                           Renderer::GenericBuffer& l,
                           Renderer::GenericBuffer& b,
                           Renderer::GenericBuffer& pressure) override;

    /**
     * @brief Iterative solving of the linear equations in data
     */
    VORTEX2D_API void Solve(Parameters& params, const std::vector<RigidBody*>& rigidbodies = {}) override;

    void Record(vk::CommandBuffer commandBuffer) override;

    /**
     * @brief Record a determined number of iterations
     * @param commandBuffer
     * @param iterations
     */
    void Record(vk::CommandBuffer commandBuffer, int iterations);

    /**
     * @brief Set the w factor of the GS iterations : x_new = w * x_new + (1-w) * x_old
     * @param w
     */
    VORTEX2D_API void SetW(float w);

    /**
     * @brief set number of iterations to be used when GS is a preconditioner
     * @param iterations
     */
    VORTEX2D_API void SetPreconditionerIterations(int iterations);

private:
    float mW;
    int mPreconditionerIterations;

    Renderer::Buffer<float> mResidual;
    Renderer::Buffer<float> mError, mLocalError;

    Renderer::Work mGaussSeidel;
    Renderer::Work::Bound mGaussSeidelBound;
    Renderer::Work mResidualWork;
    Renderer::Work::Bound mResidualBound;

    ReduceMax mReduceMax;
    ReduceMax::Bound mReduceMaxBound;

    Renderer::CommandBuffer mGaussSeidelCmd;
    Renderer::CommandBuffer mInitCmd;
    Renderer::CommandBuffer mErrorCmd;
    Renderer::GenericBuffer* mPressure;
};

/**
 * @brief A version of the gauss seidel that can only be applied on sizes (16,16) or smaller.
 */
class LocalGaussSeidel : public Preconditioner
{
public:
    VORTEX2D_API LocalGaussSeidel(const Renderer::Device& device, const glm::ivec2& size);

    void VORTEX2D_API Bind(Renderer::GenericBuffer& d,
                           Renderer::GenericBuffer& l,
                           Renderer::GenericBuffer& b,
                           Renderer::GenericBuffer& pressure) override;

    void Record(vk::CommandBuffer commandBuffer) override;

private:
    Renderer::GenericBuffer* mPressure;

    Renderer::Work mLocalGaussSeidel;
    Renderer::Work::Bound mLocalGaussSeidelBound;
};

}}

#endif
