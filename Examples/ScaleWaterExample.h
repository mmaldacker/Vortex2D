//
//  ScaleWaterExample.h
//  Vortex2D
//
//  Created by Maximilian Maldacker on 18/06/2016.
//
//

#ifndef ScaleWaterExample_h
#define ScaleWaterExample_h

#include "BaseExample.h"
#include "Water.h"


class ScaleWaterExample : public BaseExample
{
public:
    ScaleWaterExample()
    : BaseExample({glm::vec2(500), 5.0f}, 0.033)
    , gravity(glm::vec2(500))
    , obstacle(glm::vec2(100))
    , water(dimensions)
    {
        gravity.Colour = {0.0f, -0.1f, 0.0f, 0.0f};

        obstacle.Colour = glm::vec4(1);
        obstacle.Position = {200, 50};

        water.Colour = blue;

        Vortex2D::Renderer::Rectangle source({300,100});
        source.Position = {100,200};
        source.Colour = glm::vec4(1.0f);
        water.Render(source);
    }

    void Frame() override
    {
        engine.ClearBoundaries();
        engine.RenderNeumann(engine.TopBoundary);
        engine.RenderNeumann(engine.BottomBoundary);
        engine.RenderNeumann(engine.LeftBoundary);
        engine.RenderNeumann(engine.RightBoundary);

        obstacle.Colour = glm::vec4(1.0);
        engine.RenderNeumann(obstacle);

        water.RenderBoundaries(engine);

        engine.RenderForce(gravity);

        engine.Solve();

        water.Advect(engine);
    }

    void Render(Vortex2D::Renderer::RenderTarget & target) override
    {
        target.Render(water);
        obstacle.Colour = green;
        target.Render(obstacle);
    }

private:
    Vortex2D::Renderer::Rectangle gravity;
    Vortex2D::Renderer::Rectangle obstacle;
    Vortex2D::Fluid::Water water;
};

#endif /* ScaleWaterExample_h */
