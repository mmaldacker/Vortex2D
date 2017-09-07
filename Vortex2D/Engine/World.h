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
#include <Vortex2D/Engine/LinearSolver/GaussSeidel.h>
#include <Vortex2D/Engine/Size.h>
#include <Vortex2D/Engine/Extrapolation.h>
#include <Vortex2D/Engine/LevelSet.h>
#include <Vortex2D/Engine/Pressure.h>
#include <Vortex2D/Engine/Advection.h>

#include <vector>

namespace Vortex2D { namespace Fluid {

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

    void InitField(Renderer::Texture& field);

    void SolveStatic(vk::Semaphore signalSemaphore);

    void SolveDynamic();

    Renderer::RenderTexture& Velocity();
    LevelSet& LiquidPhi();
    LevelSet& SolidPhi();

private:
    Dimensions mDimensions;

    Renderer::Buffer mDiagonal;
    Renderer::Buffer mLower;
    Renderer::Buffer mDiv;
    Renderer::Buffer mPressure;

    GaussSeidel mPreconditioner;
    ConjugateGradient mLinearSolver;

    Renderer::RenderTexture mVelocity;
    Renderer::RenderTexture mBoundariesVelocity;
    LevelSet mFluidLevelSet;
    LevelSet mObstacleLevelSet;

    Renderer::Buffer mValid;

    Advection mAdvection;
    Pressure mProjection;
    Extrapolation mExtrapolation;
};

}}

#endif
