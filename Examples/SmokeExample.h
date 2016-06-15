//
//  Smoke.h
//  Vertex2D
//
//  Created by Maximilian Maldacker on 05/04/2016.
//  Copyright (c) 2016 Maximilian Maldacker. All rights reserved.
//

#ifndef Vertex2D_Smoke_h
#define Vertex2D_Smoke_h

#include "BaseExample.h"
#include "Shapes.h"
#include "Density.h"

class SmokeExample : public BaseExample
{
public:
    SmokeExample()
        : BaseExample({glm::vec2{500}, 1.0}, 0.033)
        , source1(20.0f), source2(20.0f)
        , force1(20.0f), force2(20.0f)
        , top({500,1}), bottom({500,1})
        , left({1,500}), right({1,500})
        , density(dimensions, 0.033)
    {
        top.Colour = bottom.Colour = left.Colour = right.Colour = glm::vec4{1.0f};

        top.Position = {0.0f, 0.0f};
        bottom.Position = {0.0f, 499.0f};
        left.Position = {0.0f, 0.0f};
        right.Position = {499.0f, 0.0f};

        source1.Position = force1.Position = {300.0f, 400.0f};
        source2.Position = force2.Position = {100.0f, 400.0f};

        force1.Colour = force2.Colour = {0.0f, -5.0f, 0.0f, 0.0f};

        source1.Colour = source2.Colour = glm::vec4{182.0f,172.0f,164.0f, 255.0f}/glm::vec4(255.0f);
    }

    void Frame() override
    {
        engine.RenderDirichlet(top);
        engine.RenderDirichlet(bottom);
        engine.RenderDirichlet(left);
        engine.RenderDirichlet(right);

        engine.RenderForce(force1);
        engine.RenderForce(force2);

        density.Render(source1);
        density.Render(source2);

        engine.Solve();
        density.Advect(engine);
    }

    void Render(Renderer::RenderTarget & target) override
    {
        target.Render(density);
    }

private:
    Renderer::Circle source1, source2;
    Renderer::Circle force1, force2;
    Renderer::Rectangle top, bottom, left, right;
    Fluid::Density density;
};

#endif
