//
//  ObstacleSmoke.h
//  Vertex2D
//
//  Created by Maximilian Maldacker on 05/04/2016.
//  Copyright (c) 2016 Maximilian Maldacker. All rights reserved.
//

#ifndef Vertex2D_ObstacleSmoke_h
#define Vertex2D_ObstacleSmoke_h

#include "Runner.h"
#include "Shapes.h"
#include "Sprite.h"
#include "Density.h"

const int ssize = 500;

class ObstacleSmokeExample : public Runner
{
public:
    ObstacleSmokeExample()
    : Runner({glm::vec2{ssize}, 1.0}, 0.033)
    , source(20.0f)
    , force(20.0f)
    , obstacle({100.0f, 50.0f})
    , top({ssize,1}), bottom({ssize,1})
    , left({1,ssize}), right({1,ssize})
    , density(dimensions, 0.033)
    , smoke(density.Sprite())
    {
        source.Position = force.Position = {200.0f, 400.0f};

        force.Colour = {0.0f, -5.0f, 0.0f, 0.0f};

        source.Colour = glm::vec4{182.0f,172.0f,164.0f, 255.0f}/glm::vec4(255.0f);

        obstacle.Position = {200.0f, 200.0f};
        obstacle.Rotation = 45.0f;
        obstacle.Colour = {1.0f, 0.0f, 0.0f, 1.0f};

        top.Colour = bottom.Colour = left.Colour = right.Colour = glm::vec4{1.0f};

        top.Position = {0.0f, 0.0f};
        bottom.Position = {0.0f, ssize-1.0f};
        left.Position = {0.0f, 0.0f};
        right.Position = {ssize-1.0f, 0.0f};
    }

    void frame() override
    {
        boundaries.Clear();
        boundaries.RenderNeumann({&top, &bottom, &left, &right, &obstacle});
        velocity.RenderMask(boundaries);

        velocity.Render({&force});
        density.Render({&source});

        engine.Solve();

        velocity.Advect();
        density.Advect(velocity);
    }

    std::vector<Renderer::Drawable*> render() override
    {
        return {&smoke, &obstacle};
    }

private:
    Renderer::Circle source, force;
    Renderer::Rectangle obstacle;
    Renderer::Rectangle top, bottom, left, right;
    Fluid::Density density;
    Renderer::Sprite smoke;
};

#endif
