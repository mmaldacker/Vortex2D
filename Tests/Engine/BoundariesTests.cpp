//
//  BoundariesTests.cpp
//  Vortex2D
//

#include "../Renderer/ShapeDrawer.h"
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

TEST(BoundariesTests, DISABLED_Square)
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

TEST(BoundariesTests, DISABLED_InverseSquare)
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

    PrintTexture<float>(outTexture);
    CheckLevelSet(data, outTexture);
}

void DrawSquareVelocity(int width, int height, std::vector<glm::vec2>& data, const glm::vec2& centre, const glm::vec2& size, const glm::vec2& velocity)
{
    for (int i = -size.x; i < size.x; i++)
    {
        for (int j = -size.y; j < size.y; j++)
        {
            glm::vec2 pos = glm::vec2(i, j) + centre;
            glm::vec2 upos = pos + glm::vec2(0.5f, 0.0f) - centre;
            glm::vec2 vpos = pos + glm::vec2(0.0f, 0.5f) - centre;

            int x = i + centre.x;
            int y = j + centre.y;
            data[x + y * width] = velocity;
        }
    }
}

TEST(BoundariesTests, PolygonVelocity)
{
    glm::ivec2 size(20);
    std::vector<glm::vec2> points = {{-2.0f, -2.0f},
                                     {2.0f, -2.0f},
                                     {-2.0f, 2.0f},
                                     {2.0f, -2.0f},
                                     {2.0f, 2.0f},
                                     {-2.0f, 2.0f}};

    PolygonVelocity polygon(*device, size, points, {});
    polygon.Position = {10.0f, 10.0f};
    polygon.UpdateVelocities({1.0f, 0.0f}, 0.0f);

    RenderTexture boundaryVelocity(*device, size.x, size.y, vk::Format::eR32G32Sfloat);

    boundaryVelocity.Record({polygon});
    boundaryVelocity.Submit();
    device->Handle().waitIdle();

    Texture output(*device, size.x, size.y, vk::Format::eR32G32Sfloat, true);
    ExecuteCommand(*device, [&](vk::CommandBuffer commandBuffer)
    {
        output.CopyFrom(commandBuffer, boundaryVelocity);
    });

    std::vector<glm::vec2> data(size.x*size.y);
    DrawSquareVelocity(size.x, size.y, data, polygon.Position, {2.0f, 2.0f}, {1.0f / size.x, 0.0f});

    CheckTexture(data, output);
}

glm::vec2 GetRotationVelocity(const glm::vec2& dir, float angularVelocity)
{
    return angularVelocity * glm::vec2(-dir.y, dir.x);
}

void DrawSquareRotation(int width, int height, std::vector<glm::vec2>& data, const glm::vec2& centre, const glm::vec2& size, float angularVelocity)
{
    for (int i = -size.x; i < size.x; i++)
    {
        for (int j = -size.y; j < size.y; j++)
        {
            glm::vec2 pos = glm::vec2(i, j) + centre;
            glm::vec2 upos = pos + glm::vec2(0.5f, 0.0f) - centre;
            glm::vec2 vpos = pos + glm::vec2(0.0f, 0.5f) - centre;

            int x = i + centre.x;
            int y = j + centre.y;
            data[x + y * width].x = GetRotationVelocity(upos, angularVelocity).x;
            data[x + y * width].y = GetRotationVelocity(vpos, angularVelocity).y;
        }
    }
}

TEST(BoundariesTests, PolygonVelocityRotation)
{
    glm::ivec2 size(20);
    std::vector<glm::vec2> points = {{-2.0f, -2.0f},
                                     {2.0f, -2.0f},
                                     {-2.0f, 2.0f},
                                     {2.0f, -2.0f},
                                     {2.0f, 2.0f},
                                     {-2.0f, 2.0f}};

    Buffer<glm::ivec2> valid(*device, size.x*size.y, true);

    PolygonVelocity polygon(*device, size, points, {});
    polygon.Position = {10.0f, 14.0f};
    polygon.UpdateVelocities({0.0f, 0.0f}, 1.0f);

    RenderTexture boundaryVelocity(*device, size.x, size.y, vk::Format::eR32G32Sfloat);

    boundaryVelocity.Record({polygon});
    boundaryVelocity.Submit();
    device->Handle().waitIdle();

    Texture output(*device, size.x, size.y, vk::Format::eR32G32Sfloat, true);
    ExecuteCommand(*device, [&](vk::CommandBuffer commandBuffer)
    {
        output.CopyFrom(commandBuffer, boundaryVelocity);
    });

    std::vector<glm::vec2> data(size.x*size.y);
    DrawSquareRotation(size.x, size.y, data, polygon.Position, {2.0f, 2.0f}, 1.0f / size.x);

    CheckTexture(data, output);
}
