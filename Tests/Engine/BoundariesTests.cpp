//
//  BoundariesTests.cpp
//  Vortex2D
//

#include "../Renderer/ShapeDrawer.h"
#include "Verify.h"

#include <glm/gtx/io.hpp>
#include <glm/glm.hpp>

#include <Vortex2D/Renderer/Shapes.h>
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
            glm::vec2 pos(i + 0.5, j + 0.5);
            auto p = pos - centre;
            int index = i + j * size.x;
            data[index] = glm::length(p) - radius;
        }
    }
}

float DistToSegment(glm::vec2 a, glm::vec2 b, glm::vec2 p)
{
    glm::vec2 dir = b - a;
    float l = dot(dir, dir);

    float t = glm::clamp(glm::dot(p - a, dir) / l, 0.0f, 1.0f);
    glm::vec2 proj = a + t * dir;
    return glm::distance(p, proj);
}

// +1 if is left
float Orientation(glm::vec2 a, glm::vec2 b, glm::vec2 p)
{
    float v = ((b.x - a.x) * (p.y - a.y) - (b.y - a.y) * (p.x - a.x));
    if (v >= 0.0) return 1.0;
    else return -1.0;
}

void DrawSignedSquare(const glm::ivec2& size, const std::vector<glm::vec2>& points, std::vector<float>& data, const glm::vec2& pos)
{
    for (int i = 0; i < size.x; i++)
    {
        for (int j = 0; j < size.y; j++)
        {
            int index = i + j * size.x;
            float value = (float)-std::max(size.x, size.y);
            for (std::size_t k = points.size() - 1, l = 0; l < points.size(); k = l++)
            {
                float udist = DistToSegment(points[k] + pos, points[l] + pos, glm::vec2(i + 0.5f, j + 0.5f));
                float dist = -Orientation(points[k] + pos, points[l] + pos, glm::vec2(i + 0.5f, j + 0.5f)) * udist;
                value = std::max(value, dist);
            }

            data[index] = std::min(data[index], value);
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

    Polygon square(*device, points, false, 20);
    square.Position = glm::vec2(5.0f, 10.0f);

    std::vector<float> data(size.x*size.y, 100.0f);
    DrawSignedSquare(size, points, data, square.Position);

    LevelSet levelSet(*device, size);

    Clear clear({100.0f, 0.0f, 0.0f, 0.0f});

    levelSet.Record({clear, square}).Submit();
    device->Handle().waitIdle();

    Texture outTexture(*device, size.x, size.y, vk::Format::eR32Sfloat, VMA_MEMORY_USAGE_CPU_ONLY);
    ExecuteCommand(*device, [&](vk::CommandBuffer commandBuffer)
    {
       outTexture.CopyFrom(commandBuffer, levelSet);
    });

    CheckLevelSet(data, outTexture);
}

TEST(BoundariesTests, InverseSquare)
{
    glm::ivec2 size(20);

    std::vector<glm::vec2> points = {{0.0f, 0.0f}, {4.0f, 0.0f}, {4.0f, 4.0f}, {0.0f, 4.0f}};

    Polygon square(*device, points, true, 20);
    square.Position = glm::vec2(5.0f, 10.0f);

    std::vector<float> data(size.x*size.y, 100.0f);
    DrawSignedSquare(size, points, data, square.Position);

    for (float& x: data) x *= -1.0f;

    LevelSet levelSet(*device, size);

    Clear clear({100.0f, 0.0f, 0.0f, 0.0f});

    levelSet.Record({clear, square}).Submit();
    device->Handle().waitIdle();

    Texture outTexture(*device, size.x, size.y, vk::Format::eR32Sfloat, VMA_MEMORY_USAGE_CPU_ONLY);
    ExecuteCommand(*device, [&](vk::CommandBuffer commandBuffer)
    {
       outTexture.CopyFrom(commandBuffer, levelSet);
    });

    CheckLevelSet(data, outTexture);
}

TEST(BoundariesTests, Circle)
{
    glm::ivec2 size(20);

    std::vector<glm::vec2> points = {{0.0f, 0.0f}, {4.0f, 0.0f}, {4.0f, 4.0f}, {0.0f, 4.0f}};

    Circle circle(*device, 5.0f, 20);
    circle.Position = glm::vec2(8.0f, 10.0f);

    std::vector<float> data(size.x*size.y, 100.0f);
    DrawCircle(size, data, 5.0f, circle.Position);

    LevelSet levelSet(*device, size);
    Clear clear({100.0f, 0.0f, 0.0f, 0.0f});

    levelSet.Record({clear, circle}).Submit();
    device->Handle().waitIdle();

    Texture outTexture(*device, size.x, size.y, vk::Format::eR32Sfloat, VMA_MEMORY_USAGE_CPU_ONLY);
    ExecuteCommand(*device, [&](vk::CommandBuffer commandBuffer)
    {
       outTexture.CopyFrom(commandBuffer, levelSet);
    });

    CheckLevelSet(data, outTexture);
}

TEST(BoundariesTests, Intersection)
{
    glm::ivec2 size(20);

    std::vector<glm::vec2> points = {{0.0f, 0.0f}, {4.0f, 0.0f}, {4.0f, 4.0f}, {0.0f, 4.0f}};

    Polygon square1(*device, points, false, 20);
    square1.Position = glm::vec2(5.0f, 10.0f);

    Polygon square2(*device, points, false, 20);
    square2.Position = glm::vec2(12.0f, 10.0f);

    std::vector<float> data1(size.x*size.y, 100.0f);
    DrawSignedSquare(size, points, data1, square1.Position);

    std::vector<float> data2(size.x*size.y, 100.0f);
    DrawSignedSquare(size, points, data2, square2.Position);

    std::vector<float> data(size.x*size.y, 100.0f);
    for (int i = 0; i < data.size(); i++)
    {
        data[i] = std::min(data1[i], data2[i]);
    }

    LevelSet levelSet(*device, size);
    Clear clear({100.0f, 0.0f, 0.0f, 0.0f});

    levelSet.Record({clear, square1, square2}, UnionBlend).Submit();
    device->Handle().waitIdle();

    Texture outTexture(*device, size.x, size.y, vk::Format::eR32Sfloat, VMA_MEMORY_USAGE_CPU_ONLY);
    ExecuteCommand(*device, [&](vk::CommandBuffer commandBuffer)
    {
       outTexture.CopyFrom(commandBuffer, levelSet);
    });

    CheckLevelSet(data, outTexture);
}

TEST(BoundariesTest, DistanceField)
{
    glm::ivec2 size(50);
    
    LevelSet levelSet(*device, size);
    
    std::vector<float> data(size.x*size.y, 0.1f);
    Texture localLevelSet(*device, size.x, size.y, vk::Format::eR32Sfloat, VMA_MEMORY_USAGE_CPU_ONLY);
    ExecuteCommand(*device, [&](vk::CommandBuffer commandBuffer)
    {
        localLevelSet.CopyFrom(data);
        levelSet.CopyFrom(commandBuffer, localLevelSet);
    });
    
    DistanceField distance(*device, levelSet, {1.0f, 1.0f, 1.0f, 1.0f});
    
    RenderTexture output(*device, size.x, size.y, vk::Format::eR8G8B8A8Unorm);
    Texture localOutput(*device, size.x, size.y, vk::Format::eR8G8B8A8Unorm, VMA_MEMORY_USAGE_CPU_ONLY);
    
    output.Record({distance}).Submit();
    device->Handle().waitIdle();
    
    ExecuteCommand(*device, [&](vk::CommandBuffer commandBuffer)
    {
        localOutput.CopyFrom(commandBuffer, output);
    });

    uint8_t alpha = static_cast<uint8_t>(256 * (0.1f + 0.5f));
    std::vector<glm::u8vec4> outData(size.x*size.y, {255, 255, 255, 255 - alpha});
    
    CheckTexture<glm::u8vec4>(outData, localOutput);
}
