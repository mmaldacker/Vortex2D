//
//  Water.h
//  Vertex2D
//
//  Created by Maximilian Maldacker on 05/04/2016.
//  Copyright (c) 2016 Maximilian Maldacker. All rights reserved.
//

#ifndef Vertex2D_Water_h
#define Vertex2D_Water_h

#include "BaseExample.h"
#include "LevelSet.h"
#include "Water.h"

class WaterExample : public BaseExample
{
public:
    WaterExample()
    : BaseExample({glm::vec2{500}, 1.0f}, 0.033)
        , gravity(glm::vec2{500})
        , top({500,1}), bottom({500,1})
        , left({1,500}), right({1,500})
        , obstacle1({100,100}), obstacle2({100,100})
        , levelSet(dimensions, 0.033)
        , water(levelSet)
    {
        Renderer::Disable d(GL_BLEND);

        gravity.Colour = {0.0f, 0.1f, 0.0f, 0.0f};

        top.Colour = bottom.Colour = left.Colour = right.Colour = glm::vec4{1.0f};

        top.Position = {0.0f, 0.0f};
        bottom.Position = {0.0f, 499.0f};
        left.Position = {0.0f, 0.0f};
        right.Position = {499.0f, 0.0f};

        obstacle1.Position = {150.0f, 200.0f};
        obstacle1.Rotation = 45.0f;
        obstacle1.Colour = {1.0f, 0.0f, 0.0f, 1.0f};

        obstacle2.Position = {300.0f, 300.0f};
        obstacle2.Rotation = 22.0f;
        obstacle2.Colour = {1.0f, 0.0f, 0.0f, 1.0f};

        Renderer::Rectangle source({300,100});
        source.Position = {100,50};
        source.Colour = glm::vec4{1.0};
        levelSet.Render({&source});
        levelSet.Redistance();
    }

    void frame() override
    {
        boundaries.Clear();
        boundaries.RenderNeumann({&top, &bottom, &left, &right, &obstacle1, &obstacle2});
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
        return {&water, &obstacle1, &obstacle2};
    }

private:
    Renderer::Rectangle gravity;
    Renderer::Rectangle top, bottom, left, right;
    Renderer::Rectangle obstacle1, obstacle2;
    Fluid::LevelSet levelSet;
    Fluid::Water water;
};

#endif
