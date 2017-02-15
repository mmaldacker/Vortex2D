//
//  Smoke.cpp
//  Vortex2D
//

#include <Vortex2D/Vortex2D.h>
#include <Vortex2D/Renderer/Drawable.h>

#include <functional>
#include <vector>
#include <memory>

extern glm::vec4 green;
extern glm::vec4 gray;

class SmokeExample : public Vortex2D::Renderer::Drawable
{
public:
    SmokeExample(Vortex2D::Fluid::Dimensions dimensions, float dt)
        : source1(glm::vec2(20.0f)), source2(glm::vec2(20.0f))
        , force1(glm::vec2(20.0f)), force2(glm::vec2(20.0f))
        , density(dimensions)
        , world(dimensions, dt)
    {
        source1.Position = force1.Position = {166.0f, 100.0f};
        source2.Position = force2.Position = {332.0f, 100.0f};

        force1.Colour = force2.Colour = {0.0f, 5.0f, 0.0f, 0.0f};
        source1.Colour = source2.Colour = gray;

        auto boundaries = world.DrawBoundaries();

        Vortex2D::Renderer::Rectangle area(dimensions.RealSize - glm::vec2(2.0f));
        area.Position = glm::vec2(1.0f);
        area.Colour = glm::vec4(1.0f);

        boundaries.DrawSolid(area, true);
    }

    void Render(Vortex2D::Renderer::RenderTarget & target, const glm::mat4 & transform = glm::mat4()) override
    {
        world.RenderForce(force1);
        world.RenderForce(force2);

        density.Render(source1);
        density.Render(source2);

        world.Solve();
        density.Advect(world);

        target.Render(density, transform);
    }

private:
    Vortex2D::Renderer::Ellipse source1, source2;
    Vortex2D::Renderer::Ellipse force1, force2;
    Vortex2D::Fluid::World world;
    Vortex2D::Fluid::Density density;
};
