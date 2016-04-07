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
#include "Boundaries.h"
#include "Advection.h"
#include "Size.h"
#include "Operator.h"
#include "LevelSet.h"

#include <vector>

namespace Fluid
{

class Engine
{
public:
    Engine(Dimensions dimensions, Boundaries & boundaries, Advection & advection, LinearSolver * linearSolver);
    
    void Solve();
    void Extrapolate(LevelSet & levelSet);

private:
    Dimensions mDimensions;

    LinearSolver::Data mData;
    Boundaries & mBoundaries;
    Advection & mAdvection;
    LinearSolver * mLinearSolver;

    Operator mDiv;
    Operator mProject;
    Operator mExtrapolate;
    Operator mIdentity;
};

}

#endif /* defined(__Vortex__Engine__) */
