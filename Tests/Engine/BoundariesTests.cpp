//
//  BoundariesTests.cpp
//  Vortex2D
//

#include "Verify.h"

#include <Vortex2D/Engine/LevelSet.h>
#include <Vortex2D/Engine/Boundaries.h>

using namespace Vortex2D::Renderer;
using namespace Vortex2D::Fluid;

extern Device* device;

float DistToLine(glm::vec2 a, glm::vec2 b, glm::vec2 p)
{
    glm::vec2 dir = b - a;
    glm::vec2 norm(-dir.y, dir.x);
    return glm::dot(glm::normalize(norm), a - p);
}

void DrawSignedSquareMax(const glm::ivec2& size, const std::vector<glm::vec2>& points, std::vector<float>& data, const glm::vec2& pos)
{
    for (int i = 0; i < size.x; i++)
    {
        for (int j = 0; j < size.y; j++)
        {
            int index = i + j * size.x;
            for (int k = points.size() - 1, l = 0; l < points.size(); k = l++)
            {
                data[index] = std::max(data[index], DistToLine(points[k] + pos, points[l] + pos, glm::vec2(i, j)));
            }
        }
    }
}

void DrawSignedSquareMin(const glm::ivec2& size, const std::vector<glm::vec2>& points, std::vector<float>& data, const glm::vec2& pos)
{
    for (int i = 0; i < size.x; i++)
    {
        for (int j = 0; j < size.y; j++)
        {
            int index = i + j * size.x;
            for (int k = points.size() - 1, l = 0; l < points.size(); k = l++)
            {
                data[index] = std::min(data[index], DistToLine(points[k] + pos, points[l] + pos, glm::vec2(i, j)));
            }
        }
    }
}

TEST(BoundariesTests, Square)
{
    glm::ivec2 size(20);

    std::vector<glm::vec2> points = {{0.0f, 0.0f}, {4.0f, 0.0f}, {4.0f, 4.0f}, {0.0f, 4.0f}};

    Polygon square(*device, points);
    square.Position = glm::vec2(5.0f, 10.0f);

    float maxDist = std::max(size.x, size.y);
    std::vector<float> data(size.x*size.y, -maxDist);
    DrawSignedSquareMax(size, points, data, square.Position);

    LevelSet levelSet(*device, size);

    square.Initialize(levelSet);
    square.Update({});

    device->Handle().waitIdle();

    ExecuteCommand(*device, [&](vk::CommandBuffer commandBuffer)
    {
        levelSet.Clear(commandBuffer, {{-maxDist, 0.0f, 0.0f, 0.0f}});
        square.Draw(commandBuffer, levelSet);
    });

    Texture outTexture(*device, size.x, size.y, vk::Format::eR32Sfloat, true);
    ExecuteCommand(*device, [&](vk::CommandBuffer commandBuffer)
    {
       outTexture.CopyFrom(commandBuffer, levelSet);
    });

    PrintTexture<float>(outTexture);
    CheckTexture(data, outTexture);
}

TEST(BoundariesTests, InverseSquare)
{
    glm::ivec2 size(20);

    std::vector<glm::vec2> points = {{0.0f, 4.0f}, {4.0f, 4.0f}, {4.0f, 0.0f}, {0.0f, 0.0f}};

    Rectangle square(*device, {4.0f, 4.0f}, true);
    square.Position = glm::vec2(5.0f, 10.0f);

    float maxDist = std::max(size.x, size.y);
    std::vector<float> data(size.x*size.y, maxDist);
    DrawSignedSquareMin(size, points, data, square.Position);

    LevelSet levelSet(*device, size);

    square.Initialize(levelSet);
    square.Update({});

    device->Handle().waitIdle();

    ExecuteCommand(*device, [&](vk::CommandBuffer commandBuffer)
    {
        levelSet.Clear(commandBuffer, {{maxDist, 0.0f, 0.0f, 0.0f}});
        square.Draw(commandBuffer, levelSet);
    });

    Texture outTexture(*device, size.x, size.y, vk::Format::eR32Sfloat, true);
    ExecuteCommand(*device, [&](vk::CommandBuffer commandBuffer)
    {
       outTexture.CopyFrom(commandBuffer, levelSet);
    });

    PrintTexture<float>(outTexture);
    CheckTexture(data, outTexture);
}

