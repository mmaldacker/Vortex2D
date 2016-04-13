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
#include "MarkerParticles.h"

namespace Fluid
{

class Boundaries
{
public:
    Boundaries(Dimensions dimensions);

    void RenderDirichlet(const std::vector<Renderer::Drawable*> & objects);
    void RenderNeumann(const std::vector<Renderer::Drawable*> & objects);
    void RenderFluid(MarkerParticles & markerParticles);
    void RenderVelocities(const std::vector<Renderer::Drawable*> & objects);
    void RenderMask(Buffer & mask);

    auto GetWeights()
    {
        return mWeights(mDirichletBoundaries, mNeumannBoundaries);
    }
    
    auto GetDiagonals()
    {
        return mDiagonals(mNeumannBoundaries);
    }

    void Clear();

    friend class Engine;
//private:
    Dimensions mDimensions;
    
    Buffer mDirichletBoundaries;
    Buffer mNeumannBoundaries;
    Buffer mBoundariesVelocity;

    Operator mWeights;
    Operator mDiagonals;
    Operator mBoundaryMask;
};

}

#endif /* defined(__Vertex2D__Boundaries__) */
