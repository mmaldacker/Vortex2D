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
#include "Water.h"

class WaterExample : public BaseExample
{
public:
    WaterExample()
    : BaseExample({glm::vec2(500), 1.0f}, 0.033)
        , gravity(glm::vec2{500})
        , top({500,1}), bottom({500,1})
        , left({1,500}), right({1,500})
        , obstacle1({100,100}), obstacle2({100,100})
        , water(dimensions)
    {
        Renderer::Disable d(GL_BLEND);

        gravity.Colour = {0.0f, -0.1f, 0.0f, 0.0f};

        top.Colour = bottom.Colour = left.Colour = right.Colour = glm::vec4(1.0f);

        top.Position = {0.0f, 0.0f};
        bottom.Position = {0.0f, 499.0f};
        left.Position = {0.0f, 0.0f};
        right.Position = {499.0f, 0.0f};

        obstacle1.Position = {150.0f, 200.0f};
        obstacle1.Rotation = 45.0f;

        obstacle2.Position = {250.0f, 200.0f};
        obstacle2.Rotation = 30.0f;

        water.Colour = blue;

        Renderer::Rectangle source({300,100});
        source.Position = {100,350};
        source.Colour = glm::vec4(1.0f);
        water.Render(source);
    }

    void Frame() override
    {
        engine.ClearBoundaries();
        engine.RenderNeumann(top);
        engine.RenderNeumann(bottom);
        engine.RenderNeumann(left);
        engine.RenderNeumann(right);

        obstacle1.Colour = obstacle2.Colour = glm::vec4(1.0);
        engine.RenderNeumann(obstacle1);
        engine.RenderNeumann(obstacle2);

        engine.RenderDirichlet(water.GetBoundaries());

        engine.RenderForce(gravity);

        engine.Solve();

        water.Advect(engine);
    }

    void Render(Renderer::RenderTarget & target) override
    {
        target.Render(water);
        obstacle1.Colour = obstacle2.Colour = green;
        target.Render(obstacle1);
        target.Render(obstacle2);
    }

private:
    Renderer::Rectangle gravity;
    Renderer::Rectangle top, bottom, left, right;
    Renderer::Rectangle obstacle1, obstacle2;
    Fluid::Water water;
};

#endif
