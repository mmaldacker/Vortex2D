//
//  LevelSet.h
//  Vertex2D
//
//  Created by Maximilian Maldacker on 05/04/2016.
//  Copyright (c) 2016 Maximilian Maldacker. All rights reserved.
//

#ifndef __Vertex2D__LevelSet__
#define __Vertex2D__LevelSet__

#include "Size.h"
#include "Operator.h"
#include "Drawable.h"

namespace Fluid
{

class Advection;

class LevelSet
{
public:
    LevelSet(Dimensions dimensions, float dt);

    void Render(const std::vector<Renderer::Drawable*> & objects);
    void Advect(Advection & advection);
    void Redistance();

    friend class Advection;
    friend class Water;
    friend class Boundaries;
private:
    Dimensions mDimensions;
    Buffer mLevelSet;
    Operator mRedistance;
};
    
}

#endif /* defined(__Vertex2D__LevelSet__) */
