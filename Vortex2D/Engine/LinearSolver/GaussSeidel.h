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

namespace Vortex2D { namespace Fluid {

/**
 * @brief An iterative black and red successive over relaxation linear solver.
 */
class GaussSeidel : public LinearSolver, public Preconditioner
{
public:
    GaussSeidel(const Renderer::Device& device, const glm::ivec2& size);

    void Init(Renderer::Buffer& A,
              Renderer::Buffer& b,
              Renderer::Buffer& pressure) override;

    /**
     * @brief Iterative solving of the linear equations in data
     */
    void Solve(Parameters& params) override;

    void RecordInit(vk::CommandBuffer commandBuffer) override;
    void Record(vk::CommandBuffer commandBuffer) override;

    void Record(vk::CommandBuffer commandBuffer, int iterations);
    void SetW(float w);

private:
    float mW;

    Renderer::Buffer mResidual, mError, mErrorLocal;

    Renderer::Work mGaussSeidel;
    Renderer::Work::Bound mGaussSeidelBound;
    Renderer::Work mResidualWork;
    Renderer::Work::Bound mResidualBound;

    ReduceMax mReduceMax;
    ReduceMax::Bound mReduceMaxBound;

    Renderer::CommandBuffer mGaussSeidelCmd;
    Renderer::CommandBuffer mInitCmd;
    Renderer::CommandBuffer mErrorCmd;
    Renderer::Buffer* mPressure;
};

}}

#endif
