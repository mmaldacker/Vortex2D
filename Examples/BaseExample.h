//
//  BaseExample.h
//  Vertex2D
//
//  Created by Maximilian Maldacker on 05/04/2016.
//  Copyright (c) 2016 Maximilian Maldacker. All rights reserved.
//

#ifndef Vertex2D_BaseExample_h
#define Vertex2D_BaseExample_h

#include "Size.h"
#include "Engine.h"
#include "ConjugateGradient.h"
#include "Disable.h"

class BaseExample
{
public:
    BaseExample(Fluid::Dimensions dimensions, float dt)
        : dimensions(dimensions)
        , solver(dimensions.Size)
        , engine(dimensions, &solver, dt)
    {
    }

    virtual void frame() = 0;
    virtual std::vector<Renderer::Drawable*> render() = 0;

protected:
    Fluid::Dimensions dimensions;
    Fluid::ConjugateGradient solver;
    Fluid::Engine engine;
};

#endif
