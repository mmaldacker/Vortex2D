//
//  LinearSolver.h
//  Vortex2D
//

#ifndef Vortex2D_LinearSolver_h
#define Vortex2D_LinearSolver_h

#include <Vortex2D/Renderer/Buffer.h>
#include <Vortex2D/Renderer/Texture.h>
#include <Vortex2D/Renderer/Work.h>

namespace Vortex2D { namespace Fluid {

/**
 * @brief An interface to represent a linear solver.
 */
struct LinearSolver
{
    /**
     * @brief Parameters for an iterative linear solvers.
     */
    struct Parameters
    {
        /**
         * @brief Construct parameters with max iterations and max error
         * @param iterations max number of iterations to perform
         * @param errorTolerance solver stops when the error is smaller than this.
         */
        VORTEX2D_API Parameters(unsigned iterations, float errorTolerance = 0.0f);

        /**
         * @brief Checks if we've reacched the parameters.
         * @param initialError the initial error
         * @return if we can stop the linear solver.
         */
        bool IsFinished(float initialError) const;

        unsigned Iterations;
        float ErrorTolerance;
        unsigned OutIterations;
        float OutError;
    };

    /**
     * @brief The various parts of linear equations.
     */
    struct Data
    {
        VORTEX2D_API Data(const Renderer::Device& device, const glm::ivec2& size, VmaMemoryUsage memoryUsage = VMA_MEMORY_USAGE_GPU_ONLY);

        Renderer::Buffer<float> Diagonal;
        Renderer::Buffer<glm::vec2> Lower;
        Renderer::Buffer<float> B;
        Renderer::Buffer<float> X;
    };

    virtual ~LinearSolver() {}

    /**
     * @brief Bind the buffers for the linear solver
     * @param d the diagonal of the matrxi
     * @param l the lower matrix
     * @param b the right hand side
     * @param x the unknowns
     */
    virtual void Bind(Renderer::GenericBuffer& d,
                      Renderer::GenericBuffer& l,
                      Renderer::GenericBuffer& b,
                      Renderer::GenericBuffer& x) = 0;

    /**
     * @brief Solves the linear equations
     */
    virtual void Solve(Parameters& params) = 0;
};

}}

#endif
