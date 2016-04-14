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

class VelocitySmokeExample : public BaseExample
{
public:
    VelocitySmokeExample()
    : BaseExample({glm::vec2{500}, 1.0}, 0.033)
    , obstacle({50.0f, 50.0f})
    , top({500,1}), bottom({500,1})
    , left({1,500}), right({1,500})
    , density(dimensions, 0.033)
    {
        Renderer::Rectangle source({400.0f, 50.0f});
        source.Position = {50.0f, 200.0f};
        source.Colour = glm::vec4{182.0f,172.0f,164.0f, 255.0f}/glm::vec4(255.0f);

        obstacle.Position = {200.0f, 50.0f};

        top.Colour = bottom.Colour = left.Colour = right.Colour = glm::vec4{1.0f};

        top.Position = {0.0f, 0.0f};
        bottom.Position = {0.0f, 500-1.0f};
        left.Position = {0.0f, 0.0f};
        right.Position = {500-1.0f, 0.0f};

        Renderer::Disable d(GL_BLEND);

        density.Render({&source});
    }

    void frame() override
    {
        boundaries.Clear();
        obstacle.Colour = glm::vec4{1.0f};
        boundaries.RenderNeumann({&top, &bottom, &left, &right, &obstacle});
        
        glm::vec2 pos = obstacle.Position;
        if(pos.y < 400.0f)
        {
            obstacle.Position = pos + glm::vec2{0.0f,2.0f};
            obstacle.Colour = {0.0f, 100.0f, 0.0f, 0.0f};
            boundaries.RenderVelocities({&obstacle});
        }

        velocity.RenderMask(boundaries);

        engine.Solve();

        velocity.Advect();
        density.Advect(velocity);
    }

    std::vector<Renderer::Drawable*> render() override
    {
        obstacle.Colour = {1.0f, 0.0f, 0.0f, 1.0f};
        return {&density, &obstacle};
    }

private:
    Renderer::Rectangle obstacle;
    Renderer::Rectangle top, bottom, left, right;
    Fluid::Density density;
};


#endif
