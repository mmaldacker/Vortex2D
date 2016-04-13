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

const int size = 20;

class WaterExample : public Runner
{
public:
    WaterExample()
        : Runner({glm::vec2{size}, 2.0f}, 0.033)
        , gravity(glm::vec2{size})
        , top({size,1}), bottom({size,1})
        , left({1,size}), right({1,size})
        , markerParticles(0.033)
    {
        Renderer::Disable d(GL_BLEND);

        gravity.Colour = {0.0f, 0.1f, 0.0f, 0.0f};

        top.Colour = bottom.Colour = left.Colour = right.Colour = glm::vec4{1.0f};

        top.Position = {0.0f, 0.0f};
        bottom.Position = {0.0f, size-1.0f};
        left.Position = {0.0f, 0.0f};
        right.Position = {size-1.0f, 0.0f};

        glm::vec2 pos{10.0f, 10.0f};
        Renderer::Path p;
        for(int i = 0 ; i < 6 ; i++)
            for(int j = 0 ; j < 6 ; j++)
                p.emplace_back(i+pos.x,j+pos.y);

        markerParticles.Set(p);
        
    }

    void frame() override
    {
        boundaries.Clear();
        boundaries.RenderNeumann({&top, &bottom, &left, &right});
        boundaries.RenderFluid(markerParticles);

        velocity.RenderMask(boundaries);
        velocity.Render({&gravity});

        engine.Solve();
        engine.Extrapolate();

        velocity.Advect();
        markerParticles.Advect(velocity);

        velocity.mVelocity.get().Read().Print().PrintStencil();
    }

    std::vector<Renderer::Drawable*> render() override
    {
        markerParticles.Colour = glm::vec4{182.0f,172.0f,164.0f, 255.0f}/glm::vec4(255.0f);
        return {&markerParticles};
    }

private:
    Renderer::Rectangle gravity;
    Renderer::Rectangle top, bottom, left, right;
    Fluid::MarkerParticles markerParticles;
};

#endif
