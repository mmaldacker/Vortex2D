//
//  Water.h
//  Vertex2D
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
        , gravity(glm::vec2(500))
        , obstacle1({100,100}), obstacle2({100,100})
        , water(dimensions)
    {
        gravity.Colour = {0.0f, -0.1f, 0.0f, 0.0f};

        obstacle1.Position = {150.0f, 200.0f};
        obstacle1.Rotation = 45.0f;

        obstacle2.Position = {250.0f, 200.0f};
        obstacle2.Rotation = 30.0f;

        water.Colour = blue;

        Vortex2D::Renderer::Rectangle source({300,100});
        source.Position = {100,350};
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

        obstacle1.Colour = obstacle2.Colour = glm::vec4(1.0);
        engine.RenderNeumann(obstacle1);
        engine.RenderNeumann(obstacle2);

        water.RenderBoundaries(engine);

        engine.RenderForce(gravity);

        engine.Solve();

        water.Advect(engine);
    }

    void Render(Vortex2D::Renderer::RenderTarget & target) override
    {
        target.Render(water);
        obstacle1.Colour = obstacle2.Colour = green;
        target.Render(obstacle1);
        target.Render(obstacle2);
    }

private:
    Vortex2D::Renderer::Rectangle gravity;
    Vortex2D::Renderer::Rectangle obstacle1, obstacle2;
    Vortex2D::Fluid::Water water;
};

#endif
