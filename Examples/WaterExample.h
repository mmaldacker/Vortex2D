//
//  Water.cpp
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

class WaterExample : public Vortex2D::Renderer::Drawable
{
public:
    WaterExample(Vortex2D::Fluid::Dimensions dimensions, float dt)
        : gravity(glm::vec2(500))
        , obstacle1({100,100}), obstacle2({100,100})
        , world(dimensions, dt)
    {
        gravity.Colour = {0.0f, -0.5f, 0.0f, 0.0f};

        obstacle1.Position = {150.0f, 100.0f};
        obstacle1.Rotation = 45.0f;

        obstacle2.Position = {350.0f, 100.0f};
        obstacle2.Rotation = 30.0f;

        world.Colour = blue;

        auto boundaries = world.DrawBoundaries();

        Vortex2D::Renderer::Rectangle source({300,100});
        source.Position = {100,350};
        source.Colour = glm::vec4(1.0f);
        boundaries.DrawLiquid(source);

        Vortex2D::Renderer::Rectangle area(dimensions.RealSize - glm::vec2(2.0f));
        area.Position = glm::vec2(1.0f);
        area.Colour = glm::vec4(1.0f);

        boundaries.DrawSolid(area, true);
        boundaries.DrawSolid(obstacle1);
        boundaries.DrawSolid(obstacle2);
    }

    void Render(Vortex2D::Renderer::RenderTarget & target, const glm::mat4 & transform = glm::mat4()) override
    {
        world.RenderForce(gravity);
        world.Solve();
        world.Advect();

        target.Render(world);
        target.Render(obstacle1);
        target.Render(obstacle2);
    }

private:
    Vortex2D::Renderer::Rectangle gravity;
    Vortex2D::Renderer::Rectangle obstacle1, obstacle2;
    Vortex2D::Fluid::World world;
};
