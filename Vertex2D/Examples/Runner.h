//
//  Runner.h
//  Vertex2D
//
//  Created by Maximilian Maldacker on 05/04/2016.
//  Copyright (c) 2016 Maximilian Maldacker. All rights reserved.
//

#ifndef Vertex2D_Runner_h
#define Vertex2D_Runner_h

#include "WindowRenderer.h"
#include "Size.h"
#include "Engine.h"
#include "Advection.h"
#include "Boundaries.h"
#include "ConjugateGradient.h"
#include "Disable.h"

class Runner
{
public:
    Runner(Fluid::Dimensions dimensions, float dt)
        : renderer(dimensions.Scale * dimensions.Size)
        , dimensions(dimensions)
        , solver(dimensions.Size)
        , velocity(dimensions, dt)
        , boundaries(dimensions, dt)
        , engine(dimensions, boundaries, velocity, &solver, dt)
    {
        renderer.SetBackgroundColour(glm::vec4{99.0f,96.0f,93.0f,255.0f}/glm::vec4(255.0f));
    }

    virtual void frame() = 0;
    virtual std::vector<Renderer::Drawable*> render() = 0;

    void run()
    {
        Renderer::Disable d(GL_BLEND);
        glDisable(GL_DEPTH_TEST);

        while (!renderer.ShouldClose())
        {
            glfwPollEvents();

            frame();

            Renderer::Enable d(GL_BLEND);
            renderer.Clear();
            renderer.Render(render());
            renderer.Swap();
        }
    }

protected:
    WindowRenderer renderer;

    Fluid::Dimensions dimensions;
    Fluid::ConjugateGradient solver;
    Fluid::Advection velocity;
    Fluid::Boundaries boundaries;
    Fluid::Engine engine;
};


#endif
