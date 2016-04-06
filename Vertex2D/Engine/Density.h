//
//  Density.h
//  Vertex2D
//
//  Created by Maximilian Maldacker on 05/04/2016.
//  Copyright (c) 2016 Maximilian Maldacker. All rights reserved.
//

#ifndef __Vertex2D__Density__
#define __Vertex2D__Density__

#include "Size.h"
#include "Sprite.h"
#include "Operator.h"

namespace Fluid
{

class Advection;
class Boundaries;

class Density
{
public:
    Density(Dimensions dimensions, float dt);

    void Render(const std::vector<Renderer::Drawable*> & objects);
    void RenderMask(Boundaries & boundaries);
    void Advect(Advection & advection);

    Renderer::Sprite Sprite();

private:
    Dimensions mDimensions;
    Buffer mDensity;
    Operator mAdvectDensity;
};

}

#endif /* defined(__Vertex2D__Density__) */
