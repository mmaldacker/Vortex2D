//
//  BoundariesTests.cpp
//  Vortex
//

#include "Helpers.h"

#include <Vortex2D/Renderer/Shapes.h>
#include <Vortex2D/Renderer/Disable.h>

#include <Vortex2D/Engine/Boundaries.h>

using namespace Vortex2D::Renderer;
using namespace Vortex2D::Fluid;

void CheckSameSign(const glm::vec2& size, std::vector<float>& data, Buffer& buffer)
{
    Reader reader(buffer);
    reader.Read();

    for (int i = 0; i < size.x; i++)
    {
        for (int j = 0; j < size.y; j++)
        {
            std::size_t index = i + j * size.x;
            EXPECT_GT(data[index] * reader.GetFloat(i, j), 0.0f);
        }
    }
}

TEST(BoundariesTests, Liquid)
{
    Disable d(GL_BLEND);

    glm::vec2 size(20);
    Dimensions dimensions(size, 1.0f);

    LevelSet liquidPhi(size), solidPhi(size);

    Rectangle rectangle(glm::vec2(10.0f));
    rectangle.Position = glm::vec2(5.0f);
    rectangle.Colour = glm::vec4(0.4f, 0.1f, 0.3f, 0.8f);

    {
        Boundaries boundaries(dimensions, liquidPhi, solidPhi);
        boundaries.DrawLiquid(rectangle);
    }

    std::vector<float> data(size.x * size.y, 1.0f);
    DrawSquare(size.x, size.y, data, rectangle.Position, glm::vec2(10.0f), -1.0f);

    CheckSameSign(size, data, liquidPhi);
}

TEST(BoundariesTests, Solid)
{
    Disable d(GL_BLEND);

    glm::vec2 size(20);
    glm::vec2 scaledSize = size * glm::vec2(2.0f);
    Dimensions dimensions(size, 1.0f);

    LevelSet liquidPhi(size), solidPhi(scaledSize);

    Rectangle rectangle(glm::vec2(10.0f));
    rectangle.Position = glm::vec2(5.0f);
    rectangle.Colour = glm::vec4(0.4f, 0.1f, 0.3f, 0.8f);

    {
        Boundaries boundaries(dimensions, liquidPhi, solidPhi);
        boundaries.DrawSolid(rectangle);
    }

    std::vector<float> data(scaledSize.x * scaledSize.y, 1.0f);
    DrawSquare(scaledSize.x,
               scaledSize.y,
               data,
               (glm::vec2)rectangle.Position * glm::vec2(2.0f),
               glm::vec2(10.0f) * glm::vec2(2.0f),
               -1.0f);

    CheckSameSign(scaledSize, data, solidPhi);
}

TEST(BoundariesTests, Clear)
{
    Disable d(GL_BLEND);

    glm::vec2 size(20);
    glm::vec2 scaledSize = size * glm::vec2(2.0f);
    Dimensions dimensions(size, 1.0f);

    LevelSet liquidPhi(size), solidPhi(scaledSize);

    Rectangle rectangle(glm::vec2(10.0f));
    rectangle.Position = glm::vec2(5.0f);
    rectangle.Colour = glm::vec4(0.4f, 0.1f, 0.3f, 0.8f);

    {
        Boundaries boundaries(dimensions, liquidPhi, solidPhi);
        boundaries.DrawSolid(rectangle);
        boundaries.ClearSolid();
        boundaries.DrawLiquid(rectangle);
        boundaries.ClearLiquid();
    }

    std::vector<float> liquidData(size.x * size.y, 1.0f);
    CheckSameSign(size, liquidData, liquidPhi);

    std::vector<float> solidData(scaledSize.x * scaledSize.y, 1.0f);
    CheckSameSign(scaledSize, solidData, solidPhi);
}

TEST(BoundariesTests, LiquidAndSolid)
{
    Disable d(GL_BLEND);

    glm::vec2 size(20);
    glm::vec2 scaledSize = size * glm::vec2(2.0f);
    Dimensions dimensions(size, 1.0f);

    LevelSet liquidPhi(size), solidPhi(scaledSize);

    Rectangle rectangle(glm::vec2(10.0f));
    rectangle.Position = glm::vec2(5.0f);
    rectangle.Colour = glm::vec4(0.4f, 0.1f, 0.3f, 0.8f);

    {
        Boundaries boundaries(dimensions, liquidPhi, solidPhi);

        boundaries.ClearSolid();
        boundaries.DrawSolid(rectangle);

        rectangle.Position = glm::vec2(7.0f, 5.0f);
        boundaries.DrawLiquid(rectangle);
    }

    std::vector<float> solidData(scaledSize.x * scaledSize.y, 1.0f);
    DrawSquare(scaledSize.x,
               scaledSize.y,
               solidData,
               glm::vec2(5.0f) * glm::vec2(2.0f),
               glm::vec2(10.0f) * glm::vec2(2.0f),
               -1.0f);

    CheckSameSign(scaledSize, solidData, solidPhi);

    std::vector<float> liquidPhiData(size.x * size.y, 1.0f);
    DrawSquare(size.x, size.y, liquidPhiData, glm::vec2(15.0f, 5.0f), glm::vec2(2.0f, 10.0f), -1.0f);

    CheckSameSign(size, liquidPhiData, liquidPhi);

    std::vector<float> solidPhiData(scaledSize.x * scaledSize.y, 1.0f);
    DrawSquare(scaledSize.x,
               scaledSize.y,
               solidPhiData,
               glm::vec2(5.0f) * glm::vec2(2.0f),
               glm::vec2(10.0f) * glm::vec2(2.0f),
               -1.0f);
    CheckSameSign(scaledSize, solidPhiData, solidPhi);
}

TEST(BoundariesTests, Multiple)
{
    Disable d(GL_BLEND);

    glm::vec2 size(20);
    Dimensions dimensions(size, 1.0f);

    LevelSet liquidPhi(size), solidPhi(size);

    Rectangle rectangle(glm::vec2(10.0f));
    rectangle.Position = glm::vec2(5.0f);
    rectangle.Colour = glm::vec4(0.4f, 0.1f, 0.3f, 0.8f);

    {
        Boundaries boundaries(dimensions, liquidPhi, solidPhi);
        boundaries.DrawLiquid(rectangle);
        rectangle.Position = glm::vec2(7.0f, 5.0f);
        boundaries.DrawLiquid(rectangle);
    }

    std::vector<float> data(size.x * size.y, 1.0f);
    DrawSquare(size.x, size.y, data, glm::vec2(5.0f), glm::vec2(12.0f, 10.0f), -1.0f);

    CheckSameSign(size, data, liquidPhi);
}

TEST(BoundariesTests, InvertLiquid)
{
    Disable d(GL_BLEND);

    glm::vec2 size(20);
    Dimensions dimensions(size, 1.0f);

    LevelSet liquidPhi(size), solidPhi(size);

    Rectangle rectangle(glm::vec2(10.0f));
    rectangle.Position = glm::vec2(5.0f);
    rectangle.Colour = glm::vec4(0.4f, 0.1f, 0.3f, 0.8f);

    {
        Boundaries boundaries(dimensions, liquidPhi, solidPhi);
        boundaries.DrawLiquid(rectangle, true);
    }

    std::vector<float> data(size.x * size.y, -1.0f);
    DrawSquare(size.x, size.y, data, rectangle.Position, glm::vec2(10.0f), 1.0f);

    CheckSameSign(size, data, liquidPhi);
}

TEST(BoundariesTests, InvertSolid)
{
    Disable d(GL_BLEND);

    glm::vec2 size(20);
    glm::vec2 scaledSize = size * glm::vec2(2.0f);
    Dimensions dimensions(size, 1.0f);

    LevelSet liquidPhi(size), solidPhi(scaledSize);

    Rectangle rectangle(glm::vec2(10.0f));
    rectangle.Position = glm::vec2(5.0f);
    rectangle.Colour = glm::vec4(0.4f, 0.1f, 0.3f, 0.8f);

    {
        Boundaries boundaries(dimensions, liquidPhi, solidPhi);
        boundaries.DrawSolid(rectangle, true);
    }

    std::vector<float> data(scaledSize.x * scaledSize.y, -1.0f);
    DrawSquare(scaledSize.x,
               scaledSize.y,
               data,
               (glm::vec2)rectangle.Position * glm::vec2(2.0f),
               glm::vec2(10.0f) * glm::vec2(2.0f),
               1.0f);

    CheckSameSign(scaledSize, data, solidPhi);
}

TEST(BoundariesTests, Scaled)
{
    Disable d(GL_BLEND);

    glm::vec2 size(20);
    glm::vec2 scaledSize = size * glm::vec2(2.0f);
    Dimensions dimensions(scaledSize, 2.0f);

    LevelSet liquidPhi(size), solidPhi(scaledSize);

    Rectangle rectangle(glm::vec2(10.0f));
    rectangle.Position = glm::vec2(5.0f);
    rectangle.Colour = glm::vec4(0.4f, 0.1f, 0.3f, 0.8f);

    {
        Boundaries boundaries(dimensions, liquidPhi, solidPhi);
        boundaries.DrawSolid(rectangle);
    }

    std::vector<float> data(scaledSize.x * scaledSize.y, 1.0f);
    DrawSquare(scaledSize.x,
               scaledSize.y,
               data,
               (glm::vec2)rectangle.Position,
               glm::vec2(10.0f),
               -1.0f);

    CheckSameSign(scaledSize, data, solidPhi);
}

TEST(BoundariesTests, ScaledBorders)
{
    Disable d(GL_BLEND);

    glm::vec2 size(20);
    glm::vec2 scaledSize = size * glm::vec2(2.0f);
    Dimensions dimensions(scaledSize, 2.0f);

    LevelSet liquidPhi(size), solidPhi(scaledSize);

    Rectangle area(scaledSize - glm::vec2(2.0f));
    area.Position = glm::vec2(1.0f);
    area.Colour = glm::vec4(1.0f);

    {
        Boundaries boundaries(dimensions, liquidPhi, solidPhi);
        boundaries.DrawSolid(area, true);
    }

    std::vector<float> data(scaledSize.x * scaledSize.y, 1.0f);

    for(int i = 0; i < scaledSize.x; i++)
    {
        data[i] = -1.0f;
        data[i + scaledSize.x * (scaledSize.y - 1)] = -1.0f;
    }

    for(int i = 0; i < scaledSize.y; i++)
    {
        data[scaledSize.x * i] = -1.0f;
        data[scaledSize.x - 1 + scaledSize.x * i] = -1.0f;
    }

    CheckSameSign(scaledSize, data, solidPhi);
}
