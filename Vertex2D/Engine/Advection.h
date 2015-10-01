//
//  Advection.h
//  Vertex2D
//
//  Created by Maximilian Maldacker on 08/09/2015.
//  Copyright (c) 2015 Maximilian Maldacker. All rights reserved.
//

#ifndef __Vertex2D__Advection__
#define __Vertex2D__Advection__

#include "Size.h"
#include "Operator.h"
#include "Reader.h"
#include "Boundaries.h"

namespace Fluid
{

class Engine;

class Advection
{
public:
    Advection(Dimensions dimensions, float dt);

    void RenderVelocity(const std::vector<Renderer::Drawable*> & objects);
    void RenderDensity(const std::vector<Renderer::Drawable*> & objects);
    void RenderMask(Boundaries & boundaries);

    void Advect();

    friend class Engine;
private:

    Dimensions mDimensions;
    Buffer mVelocity;
    Buffer mDensity;

    Operator mAdvect;
    Operator mAdvectDensity;
};


}

#endif /* defined(__Vertex2D__Advection__) */
