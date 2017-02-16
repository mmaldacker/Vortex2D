//
//  WorldTests.cpp
//  Vortex2D
//

#include "Helpers.h"

#include <Vortex2D/Renderer/Disable.h>

#include <Vortex2D/Engine/World.h>

#include <cmath>

using namespace Vortex2D::Renderer;
using namespace Vortex2D::Fluid;


TEST(WorldTests, Velocity)
{
    glm::vec2 size = {10.0f, 5.0f};

    Rectangle rect(size);
    rect.Position = glm::vec2(5.0f, 7.0f);
    rect.Colour = glm::vec4(0.5f);

    Dimensions dimensions(glm::vec2(30.0f), 1.0f);
    World world(dimensions, 0.01f);

    world.RenderForce(rect);

    auto& reader = world.GetVelocityReader();

    for (int i = 0; i < size.x; i++)
    {
        for (int j = 0; j < size.y; j++)
        {
            EXPECT_FLOAT_EQ(0.5f, reader.GetVec2(i + 5.0f, j + 7.0f).x);
            EXPECT_FLOAT_EQ(0.5f, reader.GetVec2(i + 5.0f, j + 7.0f).y);
        }
    }
}


TEST(WorldTests, RenderFluid)
{
    Dimensions dimensions(glm::vec2(30.0f), 1.0f);
    World world(dimensions, 0.01f);

    world.Colour = glm::vec4(1.0f);

    glm::vec2 size(10.0f, 5.0f);
    Rectangle area(size);
    area.Position = glm::vec2(4.0f);
    area.Colour = glm::vec4(1.0f);

    {
        auto boundaries = world.DrawBoundaries();
        boundaries.DrawLiquid(area);
    }

    RenderTexture texture(30, 30, Texture::PixelFormat::RGBA8888);
    world.Render(texture);

    std::vector<glm::vec4> data(30*30, glm::vec4(0.0f));
    DrawSquare(30, 30, data, area.Position, size, glm::vec4(1.0f));

    CheckTexture(30, 30, data, texture);
}

TEST(WorldTests, Solve)
{
    Dimensions dimensions(glm::vec2(30.0f), 2.0f);
    World world(dimensions, 0.01f);

    Rectangle area(dimensions.RealSize - glm::vec2(2.0f));
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

    world.Solve();

    auto& reader = world.GetVelocityReader();

    for (int i = 0; i < 15.0f; i++)
    {
        for (int j = 0; j < 15.0f; j++)
        {
            EXPECT_FALSE(std::isnan(reader.GetVec2(i, j).x));
            EXPECT_FALSE(std::isnan(reader.GetVec2(i, j).y));
        }
    }
}
