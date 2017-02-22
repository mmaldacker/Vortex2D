//
//  ObstacleSmoke.cpp
//  Vortex2D
//

#include <Vortex2D/Vortex2D.h>
#include <Vortex2D/Renderer/Drawable.h>

#include <functional>
#include <vector>
#include <memory>

extern glm::vec4 green;
extern glm::vec4 gray;
extern glm::vec4 blue;

class ObstacleSmokeExample : public Vortex2D::Renderer::Drawable
{
public:
    ObstacleSmokeExample(const glm::vec2& size, float dt)
        : source(glm::vec2(20.0f))
        , force(glm::vec2(20.0f))
        , obstacle({100.0f, 50.0f})
        , dimensions(size, 1.0f)
        , density(dimensions)
        , world(dimensions, dt)
    {
        source.Position = {200.0f, 100.0f};
        source.Colour = gray;

        force.Position = (glm::vec2)source.Position;
        force.Colour = {0.0f, 5.0f, 0.0f, 0.0f};

        obstacle.Position = {200.0f, 300.0f};
        obstacle.Rotation = 45.0f;
        obstacle.Colour = green;

        auto boundaries = world.DrawBoundaries();

        Vortex2D::Renderer::Rectangle area(size - glm::vec2(2.0f));
        area.Position = glm::vec2(1.0f);
        area.Colour = glm::vec4(1.0f);

        boundaries.DrawSolid(area, true);
        boundaries.DrawSolid(obstacle);
        boundaries.DrawLiquid(area);
    }

    void Render(Vortex2D::Renderer::RenderTarget & target, const glm::mat4 & transform = glm::mat4()) override
    {
        world.RenderForce(force);
        density.Render(source);

        world.Solve();
        density.Advect(world);

        target.Render(density, transform);
        target.Render(obstacle, transform);
    }

private:
    Vortex2D::Renderer::Ellipse source, force;
    Vortex2D::Renderer::Rectangle obstacle;
    Vortex2D::Fluid::Dimensions dimensions;
    Vortex2D::Fluid::World world;
    Vortex2D::Fluid::Density density;

};
