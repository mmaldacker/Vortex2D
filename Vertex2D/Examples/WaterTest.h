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
#include "MarkerParticles.h"
#include "LevelSet.h"
#include "Water.h"

const int size = 200;

class WaterExample : public Runner
{
public:
    WaterExample()
        : Runner({glm::vec2{size}, 1.0f}, 0.033)
        , gravity(glm::vec2{size})
        , top({size,1}), bottom({size,1})
        , left({1,size}), right({1,size})
        , obstacle({50,50})
        , levelSet(dimensions, 0.033)
        , water(levelSet)
    {
        Renderer::Disable d(GL_BLEND);

        gravity.Colour = {0.0f, 0.1f, 0.0f, 0.0f};

        top.Colour = bottom.Colour = left.Colour = right.Colour = glm::vec4{1.0f};

        top.Position = {0.0f, 0.0f};
        bottom.Position = {0.0f, size-1.0f};
        left.Position = {0.0f, 0.0f};
        right.Position = {size-1.0f, 0.0f};

        obstacle.Position = {60.0f, 120.0f};
        obstacle.Rotation = 45.0f;
        obstacle.Colour = {1.0f, 0.0f, 0.0f, 1.0f};

        Renderer::Rectangle source({70,70});
        source.Position = {20,10};
        source.Colour = glm::vec4{1.0};
        levelSet.Render({&source});
        levelSet.Redistance();
    }

    void frame() override
    {
        boundaries.Clear();
        boundaries.RenderNeumann({&top, &bottom, &left, &right, &obstacle});
        boundaries.RenderFluid(levelSet);
        velocity.RenderMask(boundaries);

        velocity.Render({&gravity});

        engine.Solve();

        velocity.Extrapolate(levelSet);

        velocity.Advect();
        levelSet.Advect(velocity);
        levelSet.Redistance();
    }

    std::vector<Renderer::Drawable*> render() override
    {
        return {&water, &obstacle};
    }

private:
    Renderer::Rectangle gravity;
    Renderer::Rectangle top, bottom, left, right;
    Renderer::Rectangle obstacle;
    Fluid::LevelSet levelSet;
    Water water;
};

#endif
