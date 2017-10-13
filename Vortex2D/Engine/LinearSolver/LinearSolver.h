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
    struct Parameters
    {
        Parameters(unsigned iterations, float errorTolerance = 0.0f);

        bool IsFinished(unsigned iterations, float error) const;

        unsigned Iterations;
        float ErrorTolerance;
        unsigned OutIterations;
        float OutError;
    };

    struct Data
    {
        Data(const Renderer::Device& device, const glm::ivec2& size, bool host = false);

        Renderer::Buffer<float> Diagonal;
        Renderer::Buffer<glm::vec2> Lower;
        Renderer::Buffer<float> B;
        Renderer::Buffer<float> X;
    };

    virtual ~LinearSolver() {}

    virtual void Init(Renderer::GenericBuffer& d,
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
