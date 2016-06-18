//
//  ObstacleSmoke.h
//  Vertex2D
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
    : BaseExample({glm::vec2(500), 1.0}, 0.033)
    , source(20.0f)
    , force(20.0f)
    , obstacle({100.0f, 50.0f})
    , density(dimensions)
    {
        source.Position = {200.0f, 100.0f};
        source.Colour = gray;

        force.Position = (glm::vec2)source.Position;
        force.Colour = {0.0f, 5.0f, 0.0f, 0.0f};

        obstacle.Position = {200.0f, 300.0f};
        obstacle.Rotation = 45.0f;

        engine.RenderNeumann(engine.TopBoundary);
        engine.RenderNeumann(engine.BottomBoundary);
        engine.RenderNeumann(engine.LeftBoundary);
        engine.RenderNeumann(engine.RightBoundary);

        obstacle.Colour = glm::vec4(1.0);
        engine.RenderNeumann(obstacle);
    }

    void Frame() override
    {
        engine.RenderForce(force);
        density.Render(source);

        engine.Solve();

        density.Advect(engine);
    }

    void Render(Vortex2D::Renderer::RenderTarget & target) override
    {
        target.Render(density);
        obstacle.Colour = green;
        target.Render(obstacle);
    }

private:
    Vortex2D::Renderer::Circle source, force;
    Vortex2D::Renderer::Rectangle obstacle;
    Vortex2D::Fluid::Density density;

};

#endif
