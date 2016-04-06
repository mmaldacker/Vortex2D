//
//  Smoke.h
//  Vertex2D
//
//  Created by Maximilian Maldacker on 05/04/2016.
//  Copyright (c) 2016 Maximilian Maldacker. All rights reserved.
//

#ifndef Vertex2D_Smoke_h
#define Vertex2D_Smoke_h

#include "Runner.h"
#include "Shapes.h"
#include "Sprite.h"
#include "Density.h"

class SmokeExample : public Runner
{
public:
    SmokeExample()
        : Runner({glm::vec2{500}, 1.0}, 0.033)
        , source({20.0f, 20.0f})
        , force({30.0f, 30.0f})
        , density(dimensions, 0.033)
        , smoke(density.Sprite())
    {
        force.Position = {400.0f, 400.0f};
        force.Colour = {-80.0f, -100.0f, 0.0f, 0.0f};

        source.Position = (glm::vec2)force.Position;
        source.Colour = glm::vec4{182.0f,172.0f,164.0f, 255.0f}/glm::vec4(255.0f);
    }

    void frame() override
    {
        boundaries.RenderBorders();

        velocity.RenderMask(boundaries);
        velocity.Render({&force});

        density.RenderMask(boundaries);
        density.Render({&source});

        engine.Solve();
        
        velocity.Advect();
        density.Advect(velocity);
    }

    std::vector<Renderer::Drawable*> render() override
    {
        return {&smoke};
    }

private:
    Renderer::Rectangle source;
    Renderer::Rectangle force;
    Fluid::Density density;
    Renderer::Sprite smoke;
};

#endif
