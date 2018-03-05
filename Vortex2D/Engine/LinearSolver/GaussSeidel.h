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
    GaussSeidel(const Renderer::Device& device, const glm::ivec2& size);

    void Bind(Renderer::GenericBuffer& d,
              Renderer::GenericBuffer& l,
              Renderer::GenericBuffer& b,
              Renderer::GenericBuffer& pressure) override;

    /**
     * @brief Iterative solving of the linear equations in data
     */
    void Solve(Parameters& params) override;

    void Record(vk::CommandBuffer commandBuffer) override;

    void Record(vk::CommandBuffer commandBuffer, int iterations);
    void SetW(float w);
    void SetPreconditionerIterations(int iterations);

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

class LocalGaussSeidel : public Preconditioner
{
public:
    LocalGaussSeidel(const Renderer::Device& device, const glm::ivec2& size);

    void Bind(Renderer::GenericBuffer& d,
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
