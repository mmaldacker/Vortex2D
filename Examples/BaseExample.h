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
#include "Advection.h"
#include "Boundaries.h"
#include "ConjugateGradient.h"
#include "Disable.h"

class BaseExample
{
public:
    BaseExample(Fluid::Dimensions dimensions, float dt)
        : dimensions(dimensions)
        , solver(dimensions.Size)
        , velocity(dimensions, dt)
        , boundaries(dimensions, dt)
        , engine(dimensions, boundaries, velocity, &solver, dt)
    {
    }

    virtual void frame() = 0;
    virtual std::vector<Renderer::Drawable*> render() = 0;

protected:
    Fluid::Dimensions dimensions;
    Fluid::ConjugateGradient solver;
    Fluid::Advection velocity;
    Fluid::Boundaries boundaries;
    Fluid::Engine engine;
};

template<typename F>
void RenderObstacle(F f)
{
    
}

template<typename T, typename...Rest, typename F>
void RenderObstacle(F f, T & o, Rest &... rest)
{
    auto colour = o.Colour;
    o.Colour = glm::vec4{1.0f};
    f(o);
    o.Colour = colour;

    RenderObstacle(f, rest...);
}

#endif
