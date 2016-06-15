//
//  ObstacleSmoke.h
//  Vertex2D
//
//  Created by Maximilian Maldacker on 05/04/2016.
//  Copyright (c) 2016 Maximilian Maldacker. All rights reserved.
//

#ifndef Vertex2D_ObstacleSmoke_h
#define Vertex2D_ObstacleSmoke_h

#include "BaseExample.h"
#include "Shapes.h"
#include "Density.h"

class ObstacleSmokeExample : public BaseExample
{
public:
    ObstacleSmokeExample()
    : BaseExample({glm::vec2{500}, 1.0}, 0.033)
    , source(20.0f)
    , force(20.0f)
    , obstacle({100.0f, 50.0f})
    , top({500,1}), bottom({500,1})
    , left({1,500}), right({1,500})
    , density(dimensions, 0.033)
    {
        source.Position = {200.0f, 400.0f};
        source.Colour = glm::vec4{182.0f,172.0f,164.0f, 255.0f}/glm::vec4(255.0f);

        force.Position = (glm::vec2)source.Position;
        force.Colour = {0.0f, -5.0f, 0.0f, 0.0f};

        obstacle.Position = {200.0f, 200.0f};
        obstacle.Rotation = 45.0f;
        obstacle.Colour = glm::vec4{35.0f, 163.0f, 143.0f, 255.0f}/glm::vec4{255.0f};

        top.Colour = bottom.Colour = left.Colour = right.Colour = glm::vec4{1.0f};

        top.Position = {0.0f, 0.0f};
        bottom.Position = {0.0f, 499.0f};
        left.Position = {0.0f, 0.0f};
        right.Position = {499.0f, 0.0f};
    }

    void Frame() override
    {
        engine.RenderNeumann(top);
        engine.RenderNeumann(bottom);
        engine.RenderNeumann(left);
        engine.RenderNeumann(right);
        engine.RenderNeumann(obstacle);

        engine.RenderForce(force);
        density.Render(source);

        engine.Solve();

        density.Advect(engine);
    }

    void Render(Renderer::RenderTarget & target) override
    {
        target.Render(density);
        target.Render(obstacle);
    }

private:
    Renderer::Circle source, force;
    Renderer::Rectangle obstacle;
    Renderer::Rectangle top, bottom, left, right;
    Fluid::Density density;
};

#endif
