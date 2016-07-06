//
//  ScaleWaterExample.h
//  Vortex2D
//

#ifndef ScaleWaterExample_h
#define ScaleWaterExample_h

#include "BaseExample.h"

class ScaleWaterExample : public BaseExample
{
public:
    ScaleWaterExample()
    : BaseExample({glm::vec2(500), 5.0f}, 0.033)
    , gravity(glm::vec2(500))
    , obstacle(glm::vec2(100))
    {
        gravity.Colour = {0.0f, -0.5f, 0.0f, 0.0f};

        obstacle.Colour = glm::vec4(1);
        obstacle.Position = {200, 50};

        engine.Colour = blue;

        Vortex2D::Renderer::Rectangle source({300,100});
        source.Position = {100,300};
        source.Colour = glm::vec4(1.0f);
        engine.RenderFluid(source);
        engine.ReinitialiseDirichlet();
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

        engine.ReinitialiseNeumann();

        engine.RenderForce(gravity);

        engine.Solve();
        engine.Advect();
    }

    void Render(Vortex2D::Renderer::RenderTarget & target) override
    {
        target.Render(engine);
        obstacle.Colour = green;
        target.Render(obstacle);
    }

private:
    Vortex2D::Renderer::Rectangle gravity;
    Vortex2D::Renderer::Rectangle obstacle;
};

#endif /* ScaleWaterExample_h */
