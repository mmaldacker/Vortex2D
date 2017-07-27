//
//  GaussSeidel.h
//  Vortex2D
//

#ifndef Vortex2D_GaussSeidel_h
#define Vortex2D_GaussSeidel_h

#include <Vortex2D/Engine/LinearSolver/LinearSolver.h>
#include <Vortex2D/Renderer/Work.h>
#include <Vortex2D/Renderer/CommandBuffer.h>

namespace Vortex2D { namespace Fluid {

/**
 * @brief An iterative black and red successive over relaxation linear solver.
 */
class GaussSeidel : public LinearSolver
{
public:
    GaussSeidel(const Renderer::Device& device, const glm::vec2& size);

    void Init(Renderer::Buffer& data, Renderer::Buffer& pressure) override;

    /**
     * @brief Iterative solving of the linear equations in data
     */
    void Solve(Parameters& params) override;

private:
    float mW;
    Renderer::Work mGaussSeidel;
    Renderer::Work::Bound mGaussSeidelBound;
    Renderer::CommandBuffer mGaussSeidelCmd;
};

}}

#endif
