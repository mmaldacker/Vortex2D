//
//  LinearSolver.h
//  Vortex2D
//

#ifndef Vortex2D_LinearSolver_h
#define Vortex2D_LinearSolver_h

#include <Vortex2D/Renderer/Buffer.h>
#include <Vortex2D/Renderer/Texture.h>
#include <Vortex2D/Renderer/Work.h>

#include <Vortex2D/Engine/LinearSolver/Reduce.h>

namespace Vortex2D
{
namespace Fluid
{
class RigidBody;

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
     * @brief Run the solver a fixed number of step or until we reached a
     * minimum error
     */
    enum class SolverType
    {
      Fixed,
      Iterative,
    };

    /**
     * @brief Construct parameters with max iterations and max error
     * @param type fixed or iterative type of solver
     * @param iterations max number of iterations to perform
     * @param errorTolerance solver stops when the error is smaller than this.
     */
    VORTEX2D_API Parameters(SolverType type, unsigned iterations, float errorTolerance = 0.0f);

    /**
     * @brief Checks if we've reacched the parameters.
     * @param initialError the initial error
     * @return if we can stop the linear solver.
     */
    bool IsFinished(float initialError) const;

    /**
     * @brief Sets the out error and out iterations to 0.
     */
    void Reset();

    SolverType Type;
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
    VORTEX2D_API Data(const Renderer::Device& device,
                      const glm::ivec2& size,
                      VmaMemoryUsage memoryUsage = VMA_MEMORY_USAGE_GPU_ONLY);

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
   * @brief Bind rigidbody with the linear solver's matrix
   * @param delta solver delta
   * @param d diagonal matrix
   * @param rigidBody rigidbody to bind to
   */
  virtual void BindRigidbody(float delta, Renderer::GenericBuffer& d, RigidBody& rigidBody) = 0;

  /**
   * @brief Solves the linear equations
   * @param params solver iteration/error parameters
   * @param rigidBodies rigidbody to include in solver's matrix
   */
  virtual void Solve(Parameters& params, const std::vector<RigidBody*>& rigidBodies = {}) = 0;

  /**
   * Calculates the max residual error of the linear system.
   */
  class Error
  {
  public:
    VORTEX2D_API Error(const Renderer::Device& device, const glm::ivec2& size);

    /**
     * @brief Bind the linear system.
     * @param d the diagonal of the matrxi
     * @param l the lower matrix
     * @param b the right hand side
     * @param x the unknowns
     */
    VORTEX2D_API void Bind(Renderer::GenericBuffer& d,
                           Renderer::GenericBuffer& l,
                           Renderer::GenericBuffer& div,
                           Renderer::GenericBuffer& pressure);

    /**
     * Submit the error calculation.
     * @return this.
     */
    VORTEX2D_API Error& Submit();

    /**
     * @brief Wait for error to be calculated.
     * @return this.
     */
    VORTEX2D_API Error& Wait();

    /**
     * @brief Get the maximum error.
     * @return The error.
     */
    VORTEX2D_API float GetError();

    // private:
    Renderer::Buffer<float> mResidual;
    Renderer::Buffer<float> mError, mLocalError;

    Renderer::Work mResidualWork;
    Renderer::Work::Bound mResidualBound;

    ReduceMax mReduceMax;
    ReduceMax::Bound mReduceMaxBound;

    Renderer::CommandBuffer mErrorCmd;
  };
};

/**
 * @brief Create a linear solver parameters object with fixed solver type
 * @param iterations number of iterations to do
 * @return parameters
 */
VORTEX2D_API LinearSolver::Parameters FixedParams(unsigned iterations);

/**
 * @brief Create a linear solver parameters object,
 * solver will continue until error tolerance is reached.
 * @param errorTolerance tolerance to reach before exiting
 * @return parameters
 */
VORTEX2D_API LinearSolver::Parameters IterativeParams(float errorTolerance);

}  // namespace Fluid
}  // namespace Vortex2D

#endif
