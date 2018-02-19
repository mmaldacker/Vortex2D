//
//  World.h
//  Vortex2D
//

#ifndef Vortex_Engine_h
#define Vortex_Engine_h

#include <Vortex2D/Renderer/Drawable.h>
#include <Vortex2D/Renderer/Shapes.h>

#include <Vortex2D/Engine/LinearSolver/LinearSolver.h>
#include <Vortex2D/Engine/LinearSolver/ConjugateGradient.h>
#include <Vortex2D/Engine/LinearSolver/Multigrid.h>
#include <Vortex2D/Engine/Size.h>
#include <Vortex2D/Engine/Extrapolation.h>
#include <Vortex2D/Engine/LevelSet.h>
#include <Vortex2D/Engine/Pressure.h>
#include <Vortex2D/Engine/Advection.h>
#include <Vortex2D/Engine/Particles.h>
#include <Vortex2D/Engine/Velocity.h>
#include <Vortex2D/Engine/Rigidbody.h>

#include <vector>
#include <memory>
#include <functional>

namespace Vortex2D { namespace Fluid {

using RigidbodyRef = std::reference_wrapper<RigidBody>;

/**
 * @brief The main class of the framework. Each instance manages a grid and this class
 * is used to set forces, define boundaries, solve the incompressbility equations and do the
 * advection.
 */
class World
{
public:
    /**
     * @brief Construct an Engine with a size, linear solver and time step.
     */
    World(const Renderer::Device& device, Dimensions dimensions, float dt);

    void InitField(Density& density);

    void SolveStatic();

    void SolveDynamic();

    Renderer::RenderTexture& Velocity();
    LevelSet& LiquidPhi();
    LevelSet& StaticSolidPhi();
    LevelSet& DynamicSolidPhi();
    Renderer::GenericBuffer& Particles();
    ParticleCount& Count();

    RigidbodyRef CreateRigidbody(ObjectDrawable& drawable, const glm::vec2& centre);

private:
    const Renderer::Device& mDevice;
    Dimensions mDimensions;

    Renderer::GenericBuffer mParticles;
    ParticleCount mParticleCount;

    Multigrid mPreconditioner;
    ConjugateGradient mLinearSolver;

    LinearSolver::Data mData;
    Fluid::Velocity mVelocity;
    LevelSet mFluidPhi;
    LevelSet mStaticSolidPhi;
    LevelSet mDynamicSolidPhi;

    Renderer::Buffer<glm::ivec2> mValid;

    Advection mAdvection;
    Pressure mProjection;
    Extrapolation mExtrapolation;

    Renderer::CommandBuffer mClearVelocity, mClearValid, mCopySolidPhi;

    std::vector<std::unique_ptr<RigidBody>> mRigidbodies;
};

}}

#endif
