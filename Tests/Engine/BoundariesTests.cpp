//
//  BoundariesTests.cpp
//  Vortex2D
//

#include "Verify.h"

#include <glm/gtx/io.hpp>

#include <Vortex2D/Engine/LevelSet.h>
#include <Vortex2D/Engine/Boundaries.h>

using namespace Vortex2D::Renderer;
using namespace Vortex2D::Fluid;

extern Device* device;


void DrawCircle(const glm::ivec2& size, std::vector<float>& data,  float radius, const glm::vec2& centre)
{
    for (int i = 0; i < size.x; i++)
    {
        for (int j = 0; j < size.y; j++)
        {
            glm::vec2 pos(i, j);
            auto p = pos - centre;
            int index = i + j * size.x;
            data[index] = glm::length(p) - radius;
        }
    }
}

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

void CheckLevelSet(const std::vector<float>& data, Vortex2D::Renderer::Texture& texture, float error = 1e-5f)
{
    std::vector<float> pixels(data.size());
    texture.CopyTo(pixels);

    for (uint32_t i = 0; i < texture.GetWidth(); i++)
    {
        for (uint32_t j = 0; j < texture.GetHeight(); j++)
        {
            float expectedValue = data[i + j * texture.GetWidth()];
            float value = pixels[i + j * texture.GetWidth()];
            EXPECT_NEAR(expectedValue, value, error) << "Value not equal at " << i << ", " << j;
        }
    }
}

TEST(BoundariesTests, Square)
{
    glm::ivec2 size(20);

    std::vector<glm::vec2> points = {{0.0f, 0.0f}, {4.0f, 0.0f}, {4.0f, 4.0f}, {0.0f, 4.0f}};

    Polygon square(*device, points);
    square.Position = glm::vec2(5.0f, 10.0f);

    std::vector<float> data(size.x*size.y, 100.0f);
    DrawSignedSquareMax(size, points, data, square.Position);

    LevelSet levelSet(*device, size);

    square.Initialize(levelSet);
    square.Update({});

    device->Handle().waitIdle();

    ExecuteCommand(*device, [&](vk::CommandBuffer commandBuffer)
    {
        levelSet.Clear(commandBuffer, std::array<float, 4>{100.0f, 0.0f, 0.0f, 0.0f});
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

    std::vector<float> data(size.x*size.y, 100.0f);
    DrawSignedSquareMin(size, points, data, square.Position);

    LevelSet levelSet(*device, size);

    square.Initialize(levelSet);
    square.Update({});

    device->Handle().waitIdle();

    ExecuteCommand(*device, [&](vk::CommandBuffer commandBuffer)
    {
        levelSet.Clear(commandBuffer, std::array<float, 4>{100.0f, 0.0f, 0.0f, 0.0f});
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

TEST(BoundariesTests, Circle)
{
    glm::ivec2 size(20);

    std::vector<glm::vec2> points = {{0.0f, 0.0f}, {4.0f, 0.0f}, {4.0f, 4.0f}, {0.0f, 4.0f}};

    Circle circle(*device, 5.0f);
    circle.Position = glm::vec2(8.0f, 10.0f);

    std::vector<float> data(size.x*size.y, 100.0f);
    DrawCircle(size, data, 5.0f, circle.Position);

    LevelSet levelSet(*device, size);

    circle.Initialize(levelSet);
    circle.Update({});

    device->Handle().waitIdle();

    ExecuteCommand(*device, [&](vk::CommandBuffer commandBuffer)
    {
        levelSet.Clear(commandBuffer, std::array<float, 4>{100.0f, 0.0f, 0.0f, 0.0f});
        circle.Draw(commandBuffer, levelSet);
    });

    Texture outTexture(*device, size.x, size.y, vk::Format::eR32Sfloat, true);
    ExecuteCommand(*device, [&](vk::CommandBuffer commandBuffer)
    {
       outTexture.CopyFrom(commandBuffer, levelSet);
    });


    Buffer vertex(*device, vk::BufferUsageFlagBits::eStorageBuffer, true, sizeof(glm::vec2));
    ExecuteCommand(*device, [&](vk::CommandBuffer commandBuffer)
    {
        vertex.CopyFrom(commandBuffer, circle.mTransformedVertices);
    });
    glm::vec2 point;
    vertex.CopyTo(point);
    std::cout << "Point " << point << std::endl;

    PrintTexture<float>(outTexture);
    CheckLevelSet(data, outTexture);
}
