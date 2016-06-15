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

    void RenderDirichlet(const Renderer::DrawablesVector & objects);

    template<typename ... Objects>
    void RenderDirichlet(Objects &... objects)
    {
        RenderObstacle([&](const Renderer::DrawablesVector & objects)
        {
            RenderDirichlet(objects);
        }, objects...);
    }

    void RenderNeumann(const std::vector<Renderer::Drawable*> & objects);

    template<typename ... Objects>
    void RenderNeumann(Objects &... objects)
    {
        RenderObstacle([&](const Renderer::DrawablesVector & objects)
        {
            RenderNeumann(objects);
        }, objects...);
    }

    void RenderVelocities(const std::vector<Renderer::Drawable*> & objects);
    void RenderForce(const std::vector<Renderer::Drawable*> & objects);
    void RenderFluid(LevelSet &levelSet);

    void Clear();

    void Solve();

    void RenderMask(Buffer & mask);
    void Advect(Fluid::Buffer & buffer);
private:
    void Extrapolate();

    template<typename F>
    void RenderObstacle(F f)
    {
    }

    template<typename F, typename Head, typename...Rest>
    void RenderObstacle(F f, Head & o, Rest &... rest)
    {
        auto colour = o.Colour;
        o.Colour = glm::vec4{1.0f};
        f({&o});
        o.Colour = colour;

        RenderObstacle(f, rest...);
    }

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
