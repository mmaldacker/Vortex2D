//
//  Engine.h
//  Vortex
//
//  Created by Maximilian Maldacker on 08/04/2014.
//
//

#ifndef __Vortex__Engine__
#define __Vortex__Engine__

#include "LinearSolver.h"
#include "Size.h"
#include "Operator.h"
#include "Shapes.h"

#include <vector>

namespace Fluid
{

class LevelSet;

/**
 * @brief The main class of the framework. Each instance manages a grid and this class
 * is used to set forces, define boundaries, solve the incompressbility equations and do the
 * advection.
 */
class Engine
{
public:
    /**
     * @brief Construct an Engine with a size, linear solver and time step.
     */
    Engine(Dimensions dimensions, LinearSolver * linearSolver, float dt);

    /**
     * @brief Render an object as a dirichlet boundary. Those are boundaries where the pressure is 0.
     * @param object Drawable needs to draw with colour (1,0,0,0)
     */
    void RenderDirichlet(Renderer::Drawable & object);

    /**
     * @brief Render an object as a neumann boudary. Those are boundaries where the pressure is the opposite of the fluid.
     * @param object Drawable needs to draw with colour (1,0,0,0)
     */
    void RenderNeumann(Renderer::Drawable & object);

    /**
     * @brief Render velocity for neumann boundary (the user has to make sure it's aligned with the neumann boundary).
     * This is used for moving objects in the fluid.
     * @param object Drawable needs to draw with colour (x,y,0,0) where (x,y) is the velocity
     */
    void RenderVelocities(Renderer::Drawable & object);

    /**
     * @brief Render a force in the fluid. For example heat pushing the fluid up or gravity pushing the water down.
     * @param object Drawable needs to draw with colour (x,y,0,0) where (x,y) is the force
     */
    void RenderForce(Renderer::Drawable & object);

    /**
     * @brief Clear the dirichlet and neumann boundaries
     */
    void ClearBoundaries();

    /**
     * @brief Clear the boundary velocities
     */
    void ClearVelocities();

    /**
     * @brief Advances by one time step. This solver the incompressible equation and does the advection.
     */
    void Solve();

    friend class Water;
    friend class Density;
private:
    void RenderMask(Buffer & mask);
    void Advect(Fluid::Buffer & buffer);
    void Extrapolate();

    Dimensions mDimensions;

    LinearSolver::Data mData;
    LinearSolver * mLinearSolver;

    Buffer mVelocity;
    Buffer mDirichletBoundaries;
    Buffer mNeumannBoundaries;
    Buffer mBoundariesVelocity;
    Buffer mExtrapolateValid;

    Operator mDiv;
    Operator mProject;
    Operator mAdvect;
    Operator mExtrapolate;
    Operator mExtrapolateMask;
    Operator mIdentity;
    Operator mWeights;
    Operator mDiagonals;
    Operator mBoundaryMask;

    Renderer::Rectangle mSurface;
};

}

#endif /* defined(__Vortex__Engine__) */
