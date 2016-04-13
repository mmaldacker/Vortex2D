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
#include "Drawable.h"

namespace Fluid
{

class Engine;
class Boundaries;

class Advection
{
public:
    Advection(Dimensions dimensions, float dt);

    void Render(const std::vector<Renderer::Drawable*> & objects);
    void RenderMask(Boundaries & boundaries);

    void Advect();
    void Advect(Buffer & buffer);

    void Extrapolate();

    friend class Engine;
//private:
    Dimensions mDimensions;
    Buffer mVelocity;

    Operator mAdvect;
    Operator mExtrapolate;
    Operator mIdentity;

};


}

#endif /* defined(__Vertex2D__Advection__) */
