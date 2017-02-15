//
//  World.h
//  Vortex2D
//

#ifndef Vortex_Engine_h
#define Vortex_Engine_h

#include <Vortex2D/Renderer/Drawable.h>
#include <Vortex2D/Renderer/Shapes.h>
#include <Vortex2D/Renderer/Operator.h>
#include <Vortex2D/Renderer/Reader.h>

#include <Vortex2D/Engine/LinearSolver/LinearSolver.h>
#include <Vortex2D/Engine/LinearSolver/ConjugateGradient.h>
#include <Vortex2D/Engine/Size.h>
#include <Vortex2D/Engine/Extrapolation.h>
#include <Vortex2D/Engine/LevelSet.h>
#include <Vortex2D/Engine/Boundaries.h>
#include <Vortex2D/Engine/Pressure.h>
#include <Vortex2D/Engine/Advection.h>

#include <vector>

namespace Vortex2D { namespace Fluid {

/**
 * @brief The main class of the framework. Each instance manages a grid and this class
 * is used to set forces, define boundaries, solve the incompressbility equations and do the
 * advection.
 */
class World : public Renderer::Drawable
{
public:
    /**
     * @brief Construct an Engine with a size, linear solver and time step.
     */
    World(Dimensions dimensions, float dt);


    Boundaries DrawBoundaries();

    /**
     * @brief Render a force in the fluid. For example heat pushing the fluid up or gravity pushing the water down.
     * @param object Drawable needs to draw with colour (x,y,0,0) where (x,y) is the force
     */
    void RenderForce(Renderer::Drawable & object);

    /**
     * @brief Advances by one time step. This solver the incompressible equation and does the advection.
     */
    void Solve();

    /**
     * @brief Renders the fluid region
     */
    void Render(Renderer::RenderTarget & target, const glm::mat4 & transform = glm::mat4()) override;

    /**
     * @brief Advect the fluid region, this is used to simulate water.
     */
    void Advect();

    Renderer::Reader& GetVelocityReader();

    /**
     * @brief the Colour to render the fluid region in
     */
    glm::vec4 Colour;

    friend class Density;
private:
    void Advect(Renderer::Buffer& buffer);

    Dimensions mDimensions;

    LinearSolver::Data mData;
    ConjugateGradient mLinearSolver;

    Renderer::Buffer mVelocity;
    Renderer::Buffer mBoundariesVelocity;
    LevelSet mFluidLevelSet;
    LevelSet mObstacleLevelSet;

    Advection mAdvection;
    Pressure mPressure;
    Extrapolation mExtrapolation;

    Renderer::Reader mVelocityReader;

    Renderer::Program mFluidProgram;
    Renderer::Uniform<glm::vec4> mColourUniform;
};

}}

#endif
