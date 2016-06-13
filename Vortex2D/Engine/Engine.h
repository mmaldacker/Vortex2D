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

class Engine
{
public:
    Engine(Dimensions dimensions, LinearSolver * linearSolver, float dt);

    void RenderDirichlet(const std::vector<Renderer::Drawable*> & objects);
    void RenderNeumann(const std::vector<Renderer::Drawable*> & objects);
    void RenderVelocities(const std::vector<Renderer::Drawable*> & objects);
    void RenderForce(const std::vector<Renderer::Drawable*> & objects);
    void RenderFluid(LevelSet &levelSet);

    void Clear();

    void Solve();

    void Advect(Fluid::Buffer & buffer);

    friend class Density;
private:
    void RenderMask(Buffer & mask);
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
