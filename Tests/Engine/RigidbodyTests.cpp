//
//  RigidbodyTests.cpp
//  Vortex2D
//

#include "Verify.h"

#include <Vortex2D/Renderer/RenderTexture.h>
#include <Vortex2D/Engine/Rigidbody.h>

using namespace Vortex2D::Renderer;
using namespace Vortex2D::Fluid;

extern Device* device;

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

TEST(RigidbodyTests, PolygonVelocity)
{
    glm::ivec2 size(20);
    std::vector<glm::vec2> points = {{-2.0f, -2.0f},
                                     {2.0f, -2.0f},
                                     {-2.0f, 2.0f},
                                     {2.0f, -2.0f},
                                     {2.0f, 2.0f},
                                     {-2.0f, 2.0f}};

    Buffer<glm::ivec2> valid(*device, size.x*size.y);

    PolygonVelocity polygon(*device, size, valid, points, {});
    polygon.Position = {10.0f, 10.0f};
    polygon.UpdateVelocities({1.0f, 0.0f}, 0.0f);

    RenderTexture boundaryVelocity(*device, size.x, size.y, vk::Format::eR32G32Sfloat);

    boundaryVelocity.Record({polygon}).Submit();

    Texture output(*device, size.x, size.y, vk::Format::eR32G32Sfloat, VMA_MEMORY_USAGE_CPU_ONLY);
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

TEST(RigidbodyTests, PolygonVelocityRotation)
{
    glm::ivec2 size(20);
    std::vector<glm::vec2> points = {{-2.0f, -2.0f},
                                     {2.0f, -2.0f},
                                     {-2.0f, 2.0f},
                                     {2.0f, -2.0f},
                                     {2.0f, 2.0f},
                                     {-2.0f, 2.0f}};

    Buffer<glm::ivec2> valid(*device, size.x*size.y);

    PolygonVelocity polygon(*device, size, valid, points, {});
    polygon.Position = {10.0f, 14.0f};
    polygon.UpdateVelocities({0.0f, 0.0f}, 1.0f);

    RenderTexture boundaryVelocity(*device, size.x, size.y, vk::Format::eR32G32Sfloat);

    boundaryVelocity.Record({polygon}).Submit();

    Texture output(*device, size.x, size.y, vk::Format::eR32G32Sfloat, VMA_MEMORY_USAGE_CPU_ONLY);
    ExecuteCommand(*device, [&](vk::CommandBuffer commandBuffer)
    {
        output.CopyFrom(commandBuffer, boundaryVelocity);
    });

    std::vector<glm::vec2> data(size.x*size.y);
    DrawSquareRotation(size.x, size.y, data, polygon.Position, {2.0f, 2.0f}, 1.0f / size.x);

    CheckTexture(data, output);
}
