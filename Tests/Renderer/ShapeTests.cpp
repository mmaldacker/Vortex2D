//
//  ShapeTests.cpp
//  Vortex2D
//

#include "ShapeDrawer.h"
#include "Verify.h"

#include <gtest/gtest.h>
#include <Vortex2D/Renderer/RenderTexture.h>
#include <Vortex2D/Renderer/RenderState.h>
#include <Vortex2D/Renderer/Shapes.h>
#include <Vortex2D/Renderer/CommandBuffer.h>

using namespace Vortex2D::Renderer;

extern Device* device;

TEST(ShapeTests, Square)
{
    glm::vec2 size = {10.0f, 20.0f};

    Rectangle rect(*device, size, glm::vec4(1.0f));
    rect.Position = glm::vec2(5.0f, 7.0f);
    rect.Scale = glm::vec2(1.5f, 1.0f);

    Clear clear(glm::vec4(0.0f));

    RenderTexture texture(*device, 50, 50, vk::Format::eR32Sfloat);
    Texture outTexture(*device, 50, 50, vk::Format::eR32Sfloat, true);

    texture.Record({clear, rect});

    texture.Submit();
    device->Queue().waitIdle();

    size *= (glm::vec2)rect.Scale;

    std::vector<float> data(50*50, 0.0f);
    DrawSquare(50, 50, data, rect.Position, size, 1.0f);

    ExecuteCommand(*device, [&](vk::CommandBuffer commandBuffer)
    {
        outTexture.CopyFrom(commandBuffer, texture);
    });

    CheckTexture(data, outTexture);
}

TEST(ShapeTests, IntSquare)
{
    glm::vec2 size = {10.0f, 20.0f};

    IntRectangle rect(*device, size, glm::ivec4(1));
    rect.Position = glm::vec2(5.0f, 7.0f);

    RenderTexture texture(*device, 50, 50, vk::Format::eR32Sint);
    Texture outTexture(*device, 50, 50, vk::Format::eR32Sint, true);

    texture.Record({rect});

    texture.Submit();
    device->Queue().waitIdle();

    std::vector<int> data(50*50, 0.0f);
    DrawSquare(50, 50, data, rect.Position, size, 1);

    ExecuteCommand(*device, [&](vk::CommandBuffer commandBuffer)
    {
        outTexture.CopyFrom(commandBuffer, texture);

    });

    CheckTexture(data, outTexture);
}

TEST(ShapeTests, MultipleSquares)
{
    glm::vec2 size = {10.0f, 20.0f};

    Rectangle rect1(*device, size, glm::vec4(1.0f));
    rect1.Position = glm::vec2(5.0f, 7.0f);
    rect1.Scale = glm::vec2(1.5f, 1.0f);

    Rectangle rect2(*device, size, glm::vec4(1.0f));
    rect2.Position = glm::vec2(20.0f, 27.0f);

    Clear clear(glm::vec4(0.0f));

    RenderTexture texture(*device, 50, 50, vk::Format::eR32Sfloat);
    Texture outTexture(*device, 50, 50, vk::Format::eR32Sfloat, true);

    texture.Record({clear, rect1});

    texture.Submit();
    device->Queue().waitIdle();

    texture.Record({rect2});

    texture.Submit();
    device->Queue().waitIdle();

    std::vector<float> data(50*50, 0.0f);
    DrawSquare(50, 50, data, rect2.Position, size, 1.0f);

    size *= (glm::vec2)rect1.Scale;
    DrawSquare(50, 50, data, rect1.Position, size, 1.0f);

    ExecuteCommand(*device, [&](vk::CommandBuffer commandBuffer)
    {
       outTexture.CopyFrom(commandBuffer, texture);
    });

    CheckTexture(data, outTexture);
}

TEST(ShapeTests, Circle)
{
    Ellipse ellipse(*device, glm::vec2(5.0f), glm::vec4(1.0f));
    ellipse.Position = glm::vec2(10.0f, 15.0f);

    Clear clear(glm::vec4(0.0f));

    RenderTexture texture(*device, 50, 50, vk::Format::eR32Sfloat);
    Texture outTexture(*device, 50, 50, vk::Format::eR32Sfloat, true);

    texture.Record({clear, ellipse});

    texture.Submit();
    device->Queue().waitIdle();

    std::vector<float> data(50*50, 0.0f);
    DrawCircle(50, 50, data, ellipse.Position, 5.0f);

    ExecuteCommand(*device, [&](vk::CommandBuffer commandBuffer)
    {
       outTexture.CopyFrom(commandBuffer, texture);
    });

    CheckTexture(data, outTexture);
}

TEST(ShapeTests, Ellipse)
{
    glm::vec2 radius(4.0f, 7.0f);

    Ellipse ellipse(*device, radius, glm::vec4(1.0f));
    ellipse.Position = glm::vec2(20.0f, 15.0f);

    Clear clear(glm::vec4(0.0f));

    RenderTexture texture(*device, 50, 50, vk::Format::eR32Sfloat);
    Texture outTexture(*device, 50, 50, vk::Format::eR32Sfloat, true);

    texture.Record({clear, ellipse});

    texture.Submit();
    device->Queue().waitIdle();

    std::vector<float> data(50*50, 0.0f);
    DrawEllipse(50, 50, data, ellipse.Position, radius);

    ExecuteCommand(*device, [&](vk::CommandBuffer commandBuffer)
    {
       outTexture.CopyFrom(commandBuffer, texture);
    });

    CheckTexture(data, outTexture);
}

TEST(ShapeTests, ScaledEllipse)
{
    glm::vec2 pos(20.0f, 15.0f);
    glm::vec2 radius(4.0f, 7.0f);

    Ellipse ellipse(*device, radius, glm::vec4(1.0f));
    ellipse.Position = pos;
    ellipse.Scale = glm::vec2(1.0f, 2.0f);

    Clear clear(glm::vec4(0.0f));

    RenderTexture texture(*device, 50, 50, vk::Format::eR32Sfloat);
    Texture outTexture(*device, 50, 50, vk::Format::eR32Sfloat, true);

    texture.Record({clear, ellipse});

    texture.Submit();
    device->Queue().waitIdle();

    radius *= (glm::vec2)ellipse.Scale;
    std::vector<float> data(50*50, 0.0f);
    DrawEllipse(50, 50, data, ellipse.Position, radius);

    ExecuteCommand(*device, [&](vk::CommandBuffer commandBuffer)
    {
       outTexture.CopyFrom(commandBuffer, texture);
    });

    CheckTexture(data, outTexture);
}

TEST(ShapeTests, RotatedEllipse)
{
    glm::vec2 radius(4.0f, 7.0f);

    Ellipse ellipse(*device, radius, glm::vec4(1.0f));
    ellipse.Position = glm::vec2(20.0f, 15.0f);
    ellipse.Rotation = 33.0f;

    Clear clear(glm::vec4(0.0f));

    RenderTexture texture(*device, 50, 50, vk::Format::eR32Sfloat);
    Texture outTexture(*device, 50, 50, vk::Format::eR32Sfloat, true);

    texture.Record({clear, ellipse});

    texture.Submit();
    device->Queue().waitIdle();

    std::vector<float> data(50*50, 0.0f);
    DrawEllipse(50, 50, data, ellipse.Position, radius, ellipse.Rotation);

    ExecuteCommand(*device, [&](vk::CommandBuffer commandBuffer)
    {
       outTexture.CopyFrom(commandBuffer, texture);
    });

    CheckTexture(data, outTexture);
}

TEST(ShapeTests, RenderScaledEllipse)
{
    glm::vec2 pos(10.0f, 10.0f);
    glm::vec2 radius(5.0f, 8.0f);

    Ellipse ellipse(*device, radius, glm::vec4(1.0f));
    ellipse.Position = pos;

    Clear clear(glm::vec4(0.0f));

    RenderTexture texture(*device, 50, 50, vk::Format::eR32Sfloat);
    Texture outTexture(*device, 50, 50, vk::Format::eR32Sfloat, true);

    texture.View = glm::scale(glm::vec3(2.0f, 2.0f, 1.0f));
    texture.Record({clear, ellipse});
    texture.Submit();
    device->Queue().waitIdle();

    radius *= glm::vec2(2.0f);
    pos *= glm::vec2(2.0f);
    std::vector<float> data(50*50, 0.0f);
    DrawEllipse(50, 50, data, pos, radius);

    ExecuteCommand(*device, [&](vk::CommandBuffer commandBuffer)
    {
       outTexture.CopyFrom(commandBuffer, texture);
    });

    CheckTexture(data, outTexture);
}
