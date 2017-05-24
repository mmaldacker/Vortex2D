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

using namespace Vortex2D::Renderer;

extern Device* device;

TEST(RenderingTest, Square)
{
    glm::vec2 size = {10.0f, 20.0f};

    Rectangle rect(*device, size, glm::vec4(1.0f));
    rect.Position = glm::vec2(5.0f, 7.0f);
    rect.Scale = glm::vec2(1.5f, 1.0f);

    RenderTexture texture(*device, 50, 50, vk::Format::eR32Sfloat);
    Texture outTexture(*device, 50, 50, vk::Format::eR32Sfloat, true);

    rect.Initialize({texture});
    rect.Update(texture.Orth, glm::mat4());

    texture.Record([&](vk::CommandBuffer commandBuffer)
    {
        Clear(50, 50, glm::vec4(0.0f)).Draw(commandBuffer);
        rect.Draw(commandBuffer, {texture});
    });

    texture.Submit();
    device->Queue().waitIdle();

    size *= (glm::vec2)rect.Scale;

    std::vector<float> data(50*50, 0.0f);
    DrawSquare(50, 50, data, rect.Position, size, 1.0f);

    device->ExecuteCommand([&](vk::CommandBuffer commandBuffer)
    {
       outTexture.CopyFrom(commandBuffer, texture);
       outTexture.Barrier(commandBuffer, vk::ImageLayout::eGeneral, vk::AccessFlagBits::eHostRead);
    });

    CheckTexture(data, outTexture, 4);
}

TEST(RenderingTest, Circle)
{
    Ellipse ellipse(*device, glm::vec2(5.0f), glm::vec4(1.0f));
    ellipse.Position = glm::vec2(10.0f, 15.0f);

    RenderTexture texture(*device, 50, 50, vk::Format::eR32Sfloat);
    Texture outTexture(*device, 50, 50, vk::Format::eR32Sfloat, true);

    ellipse.Initialize({texture});
    ellipse.Update(texture.Orth, glm::mat4());

    texture.Record([&](vk::CommandBuffer commandBuffer)
    {
        Clear(50, 50, glm::vec4(0.0f)).Draw(commandBuffer);
        ellipse.Draw(commandBuffer, {texture});
    });

    texture.Submit();
    device->Queue().waitIdle();

    std::vector<float> data(50*50, 0.0f);
    DrawCircle(50, 50, data, ellipse.Position, 5.0f);

    device->ExecuteCommand([&](vk::CommandBuffer commandBuffer)
    {
       outTexture.CopyFrom(commandBuffer, texture);
       outTexture.Barrier(commandBuffer, vk::ImageLayout::eGeneral, vk::AccessFlagBits::eHostRead);
    });

    CheckTexture(data, outTexture, 4);
}

TEST(RenderingTest, Ellipse)
{
    glm::vec2 radius(4.0f, 7.0f);

    Ellipse ellipse(*device, radius, glm::vec4(1.0f));
    ellipse.Position = glm::vec2(20.0f, 15.0f);

    RenderTexture texture(*device, 50, 50, vk::Format::eR32Sfloat);
    Texture outTexture(*device, 50, 50, vk::Format::eR32Sfloat, true);

    ellipse.Initialize({texture});
    ellipse.Update(texture.Orth, glm::mat4());

    texture.Record([&](vk::CommandBuffer commandBuffer)
    {
        Clear(50, 50, glm::vec4(0.0f)).Draw(commandBuffer);
        ellipse.Draw(commandBuffer, {texture});
    });

    texture.Submit();
    device->Queue().waitIdle();

    std::vector<float> data(50*50, 0.0f);
    DrawEllipse(50, 50, data, ellipse.Position, radius);

    device->ExecuteCommand([&](vk::CommandBuffer commandBuffer)
    {
       outTexture.CopyFrom(commandBuffer, texture);
       outTexture.Barrier(commandBuffer, vk::ImageLayout::eGeneral, vk::AccessFlagBits::eHostRead);
    });

    CheckTexture(data, outTexture, 4);
}

TEST(RenderingTest, ScaledEllipse)
{
    glm::vec2 pos(20.0f, 15.0f);
    glm::vec2 radius(4.0f, 7.0f);

    Ellipse ellipse(*device, radius, glm::vec4(1.0f));
    ellipse.Position = pos;
    ellipse.Scale = glm::vec2(1.0f, 2.0f);

    RenderTexture texture(*device, 50, 50, vk::Format::eR32Sfloat);
    Texture outTexture(*device, 50, 50, vk::Format::eR32Sfloat, true);

    ellipse.Initialize({texture});
    ellipse.Update(texture.Orth, glm::mat4());

    texture.Record([&](vk::CommandBuffer commandBuffer)
    {
        Clear(50, 50, glm::vec4(0.0f)).Draw(commandBuffer);
        ellipse.Draw(commandBuffer, {texture});
    });

    texture.Submit();
    device->Queue().waitIdle();

    radius *= (glm::vec2)ellipse.Scale;
    std::vector<float> data(50*50, 0.0f);
    DrawEllipse(50, 50, data, ellipse.Position, radius);

    device->ExecuteCommand([&](vk::CommandBuffer commandBuffer)
    {
       outTexture.CopyFrom(commandBuffer, texture);
       outTexture.Barrier(commandBuffer, vk::ImageLayout::eGeneral, vk::AccessFlagBits::eHostRead);
    });

    CheckTexture(data, outTexture, 4);
}

TEST(RenderingTest, RotatedEllipse)
{
    glm::vec2 radius(4.0f, 7.0f);

    Ellipse ellipse(*device, radius, glm::vec4(1.0f));
    ellipse.Position = glm::vec2(20.0f, 15.0f);
    ellipse.Rotation = 33.0f;

    RenderTexture texture(*device, 50, 50, vk::Format::eR32Sfloat);
    Texture outTexture(*device, 50, 50, vk::Format::eR32Sfloat, true);

    ellipse.Initialize({texture});
    ellipse.Update(texture.Orth, glm::mat4());

    texture.Record([&](vk::CommandBuffer commandBuffer)
    {
        Clear(50, 50, glm::vec4(0.0f)).Draw(commandBuffer);
        ellipse.Draw(commandBuffer, {texture});
    });

    texture.Submit();
    device->Queue().waitIdle();

    std::vector<float> data(50*50, 0.0f);
    DrawEllipse(50, 50, data, ellipse.Position, radius, ellipse.Rotation);

    device->ExecuteCommand([&](vk::CommandBuffer commandBuffer)
    {
       outTexture.CopyFrom(commandBuffer, texture);
       outTexture.Barrier(commandBuffer, vk::ImageLayout::eGeneral, vk::AccessFlagBits::eHostRead);
    });

    CheckTexture(data, outTexture, 4);
}

TEST(RenderingTest, RenderScaledEllipse)
{
    glm::vec2 pos(10.0f, 10.0f);
    glm::vec2 radius(5.0f, 8.0f);

    Ellipse ellipse(*device, radius, glm::vec4(1.0f));
    ellipse.Position = pos;

    RenderTexture texture(*device, 50, 50, vk::Format::eR32Sfloat);
    Texture outTexture(*device, 50, 50, vk::Format::eR32Sfloat, true);

    ellipse.Initialize({texture});
    ellipse.Update(texture.Orth, glm::scale(glm::vec3(2.0f, 2.0f, 1.0f)));

    texture.Record([&](vk::CommandBuffer commandBuffer)
    {
        Clear(50, 50, glm::vec4(0.0f)).Draw(commandBuffer);
        ellipse.Draw(commandBuffer, {texture});
    });

    texture.Submit();
    device->Queue().waitIdle();

    radius *= glm::vec2(2.0f);
    pos *= glm::vec2(2.0f);
    std::vector<float> data(50*50, 0.0f);
    DrawEllipse(50, 50, data, pos, radius);

    device->ExecuteCommand([&](vk::CommandBuffer commandBuffer)
    {
       outTexture.CopyFrom(commandBuffer, texture);
       outTexture.Barrier(commandBuffer, vk::ImageLayout::eGeneral, vk::AccessFlagBits::eHostRead);
    });

    CheckTexture(data, outTexture, 4);
}
