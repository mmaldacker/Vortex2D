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
    WaterExample(const glm::vec2& size, float dt)
        : gravity(size)
        , obstacle1({100,100}), obstacle2({100,100})
        , dimensions(size, 1.0f)
        , world(dimensions, dt)
    {
        gravity.Colour = {0.0f, -0.5f, 0.0f, 0.0f};

        obstacle1.Position = {150.0f, 100.0f};
        obstacle1.Rotation = 45.0f;
        obstacle1.Colour = green;

        obstacle2.Position = {350.0f, 100.0f};
        obstacle2.Rotation = 30.0f;
        obstacle2.Colour = green;

        world.Colour = blue;

        auto boundaries = world.DrawBoundaries();

        Vortex2D::Renderer::Rectangle source({300,100});
        source.Position = {100,350};
        source.Colour = glm::vec4(1.0f);
        boundaries.DrawLiquid(source);

        Vortex2D::Renderer::Rectangle area(size - glm::vec2(2.0f));
        area.Position = glm::vec2(1.0f);
        area.Colour = glm::vec4(1.0f);

        boundaries.DrawSolid(area, true);
        boundaries.DrawSolid(obstacle1);
        boundaries.DrawSolid(obstacle2);
    }

    void Render(const Vortex2D::Renderer::Device& device, Vortex2D::Renderer::RenderTarget & target) override
    {
        /*
        world.RenderForce(gravity);
        world.Solve();
        world.Advect();

        target.Render(world);
        target.Render(obstacle1);
        target.Render(obstacle2);
        */
    }

private:
    Vortex2D::Renderer::Rectangle gravity;
    Vortex2D::Renderer::Rectangle obstacle1, obstacle2;
    Vortex2D::Fluid::Dimensions dimensions;
    Vortex2D::Fluid::World world;
};
