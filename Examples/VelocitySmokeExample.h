//
//  VelocitySmokeTest.h
//  Vertex2D
//
//  Created by Maximilian Maldacker on 06/04/2016.
//  Copyright (c) 2016 Maximilian Maldacker. All rights reserved.
//

#ifndef Vertex2D_VelocitySmokeTest_h
#define Vertex2D_VelocitySmokeTest_h


#include "BaseExample.h"
#include "Shapes.h"
#include "Density.h"

const float speed = 2.0f;

class VelocitySmokeExample : public BaseExample
{
public:
    VelocitySmokeExample()
    : BaseExample({glm::vec2{500}, 1.0}, 0.033)
    , obstacle({50.0f, 50.0f}), force({50.0f, 50.0f})
    , top({500,1}), bottom({500,1})
    , left({1,500}), right({1,500})
    , density(dimensions, 0.033)
    {
        obstacle.Position = {200.0f, 50.0f};
        obstacle.Colour = glm::vec4{35.0f, 163.0f, 143.0f, 255.0f}/glm::vec4{255.0f};

        force.Position = (glm::vec2)obstacle.Position;
        force.Colour = {0.0f, speed/0.033f, 0.0f, 0.0f};

        top.Colour = bottom.Colour = left.Colour = right.Colour = glm::vec4{1.0f};

        top.Position = {0.0f, 0.0f};
        bottom.Position = {0.0f, 500-1.0f};
        left.Position = {0.0f, 0.0f};
        right.Position = {500-1.0f, 0.0f};

        Renderer::Disable d(GL_BLEND);

        Renderer::Rectangle source({400.0f, 50.0f});
        source.Position = {50.0f, 200.0f};
        source.Colour = glm::vec4{182.0f,172.0f,164.0f, 255.0f}/glm::vec4(255.0f);
        density.Render(source);
    }

    void Frame() override
    {
        engine.ClearBoundaries();
        engine.RenderNeumann(top);
        engine.RenderNeumann(bottom);
        engine.RenderNeumann(left);
        engine.RenderNeumann(right);
        engine.RenderNeumann(obstacle);

        glm::vec2 pos = obstacle.Position;
        if(pos.y < 400.0f)
        {
            obstacle.Position = force.Position = pos + glm::vec2{0.0f,speed};
            engine.RenderVelocities(force);
        }

        engine.Solve();
        density.Advect(engine);
    }

    void Render(Renderer::RenderTarget & target) override
    {
        target.Render(density);
        target.Render(obstacle);
    }

private:
    Renderer::Rectangle obstacle, force;
    Renderer::Rectangle top, bottom, left, right;
    Fluid::Density density;
};


#endif
