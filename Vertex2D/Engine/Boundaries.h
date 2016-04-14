//
//  Boundaries.h
//  Vertex2D
//
//  Created by Maximilian Maldacker on 01/09/2015.
//  Copyright (c) 2015 Maximilian Maldacker. All rights reserved.
//

#ifndef __Vertex2D__Boundaries__
#define __Vertex2D__Boundaries__

#include "Size.h"
#include "Operator.h"
#include "LevelSet.h"

namespace Fluid
{

class Boundaries
{
public:
    Boundaries(Dimensions dimensions, float dt);

    void RenderDirichlet(const std::vector<Renderer::Drawable*> & objects);
    void RenderNeumann(const std::vector<Renderer::Drawable*> & objects);
    void RenderFluid(LevelSet & levelSet);
    void RenderVelocities(const std::vector<Renderer::Drawable*> & objects);
    void RenderMask(Buffer & mask);

    Context GetWeights();
    Context GetDiagonals();

    void Clear();

    friend class Engine;
private:
    Dimensions mDimensions;
    
    Buffer mDirichletBoundaries;
    Buffer mNeumannBoundaries;
    Buffer mBoundariesVelocity;

    Operator mWeights;
    Operator mDiagonals;
    Operator mBoundaryMask;
    Operator mLevelSetMask;
};

}

#endif /* defined(__Vertex2D__Boundaries__) */
