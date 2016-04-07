//
//  Water.h
//  Vertex2D
//
//  Created by Maximilian Maldacker on 05/04/2016.
//  Copyright (c) 2016 Maximilian Maldacker. All rights reserved.
//

#ifndef Vertex2D_Water_h
#define Vertex2D_Water_h

#include "Runner.h"
#include "LevelSet.h"
#include "Shapes.h"
#include "Water.h"

class WaterExample : public Runner
{
public:
    WaterExample()
        : Runner({glm::vec2{500}, 1.0f}, 0.033)
        , source({50.0f, 50.0f})
        , gravity(dimensions.Size)
        , levelSet(dimensions, 0.033)
        , water(levelSet)
    {
        Renderer::Disable d(GL_BLEND);

        source.Position = {100.0f, 400.0f};
        source.Colour = {1.0f, 1.0f, 1.0f, 1.0f};

        gravity.Colour = {0.0f, 0.1f, 0.0f, 0.0f};

        levelSet.Render({&source});
        levelSet.Redistance();
    }

    void frame() override
    {
        boundaries.RenderBorders();
        boundaries.RenderLevelSet(levelSet);

        levelSet.Redistance();

        engine.Extrapolate(levelSet);

        velocity.RenderMask(boundaries);
        velocity.Render({&gravity});

        velocity.Advect();
        levelSet.Advect(velocity);

        engine.Solve();
    }

    std::vector<Renderer::Drawable*> render() override
    {
        return {&water};
    }

private:
    Renderer::Rectangle source, gravity;
    Fluid::LevelSet levelSet;
    Water water;
};

#endif
