//
//  AdvectionTests.cpp
//  Vortex2D
//

#include "Helpers.h"

#include <Vortex2D/Renderer/Disable.h>

#include <Vortex2D/Engine/World.h>

using namespace Vortex2D::Renderer;
using namespace Vortex2D::Fluid;

TEST(WorldTests, Velocity)
{
    glm::vec2 size = {10.0f, 5.0f};

    Rectangle rect(size);
    rect.Position = glm::vec2(5.0f, 7.0f);
    rect.Colour = glm::vec4(0.5f);

    Dimensions dimensions(glm::vec2(30.0f), 2.0f);
    World world(dimensions, 0.01f);

    world.RenderForce(rect);

    world.GetVelocityReader().Print();

    // FIXME assert test
}

TEST(WorldTests, Solve)
{
    Dimensions dimensions(glm::vec2(30.0f), 1.0f);
    World world(dimensions, 0.01f);

    Vortex2D::Renderer::Rectangle area(dimensions.RealSize - glm::vec2(2.0f));
    area.Position = glm::vec2(1.0f);
    area.Colour = glm::vec4(1.0f);

    {
        auto boundaries = world.DrawBoundaries();
        boundaries.DrawSolid(area, true);
        boundaries.DrawLiquid(area);
    }

    Rectangle rect(glm::vec2(5.0f));
    rect.Position = glm::vec2(5.0f, 7.0f);
    rect.Colour = glm::vec4(0.5f);
    world.RenderForce(rect);

    world.GetVelocityReader().Print();

    world.Solve();

    world.GetVelocityReader().Print();

    // FIXME assert test
}
