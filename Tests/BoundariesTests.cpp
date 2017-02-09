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

        boundaries.ClearLiquid();
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
    Dimensions dimensions(size, 1.0f);

    LevelSet liquidPhi(size), solidPhi(size*glm::vec2(2.0f));

    Rectangle rectangle(glm::vec2(10.0f));
    rectangle.Position = glm::vec2(5.0f);
    rectangle.Colour = glm::vec4(0.4f, 0.1f, 0.3f, 0.8f);

    {
        Boundaries boundaries(dimensions, liquidPhi, solidPhi);

        boundaries.ClearSolid();
        boundaries.DrawSolid(rectangle);
    }

    std::vector<float> data(4.0f * size.x * size.y, 1.0f);
    DrawSquare(size.x, size.y, data, rectangle.Position, glm::vec2(10.0f), -1.0f);

    Reader(solidPhi).Read().Print();
    PrintData(size.x, size.y, data);

    //CheckSameSign(size, data, solidPhi);
}

