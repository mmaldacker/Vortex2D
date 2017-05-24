//
//  RenderingTests.cpp
//  Vortex2D
//

#include <gtest/gtest.h>

#include <Vortex2D/Renderer/Shapes.h>
#include <Vortex2D/Renderer/Texture.h>
#include <Vortex2D/Renderer/RenderTexture.h>

#include "ShapeDrawer.h"
#include "Verify.h"

using namespace Vortex2D::Renderer;

extern Device* device;

TEST(RenderingTest, WriteHostTextureInt)
{
    Texture texture(*device, 50, 50, vk::Format::eR8Uint, true);

    std::vector<uint8_t> data(50*50, 0);
    DrawSquare<uint8_t>(50, 50, data, glm::vec2(10.0f, 15.0f), glm::vec2(5.0f, 8.0f), 12);

    texture.CopyFrom(data.data(), 1);

    CheckTexture(data, texture, 1);
}

TEST(RenderingTest, WriteHostTextureFloat)
{
    Texture texture(*device, 50, 50, vk::Format::eR32Sfloat, true);

    std::vector<float> data(50*50, 0.0f);
    DrawSquare(50, 50, data, glm::vec2(10.0f, 15.0f), glm::vec2(5.0f, 8.0f), 1.0f);

    texture.CopyFrom(data.data(), 4);

    CheckTexture(data, texture, 4);
}

TEST(RenderingTest, TextureCopy)
{
    Texture texture(*device, 50, 50, vk::Format::eR32Sint, false);
    Texture inTexture(*device, 50, 50, vk::Format::eR32Sint, true);
    Texture outTexture(*device, 50, 50, vk::Format::eR32Sint, true);

    std::vector<int8_t> data(50*50, 0);
    DrawSquare<int8_t>(50, 50, data, glm::vec2(10.0f, 15.0f), glm::vec2(5.0f, 8.0f), -5);

    inTexture.CopyFrom(data.data(), 1);

    device->ExecuteCommand([&](vk::CommandBuffer commandBuffer)
    {
       texture.CopyFrom(commandBuffer, inTexture);
    });

    device->ExecuteCommand([&](vk::CommandBuffer commandBuffer)
    {
       outTexture.CopyFrom(commandBuffer, texture);
       outTexture.Barrier(commandBuffer, vk::ImageLayout::eGeneral, vk::AccessFlagBits::eHostRead);
    });

    CheckTexture(data, outTexture, 1);
}

TEST(RenderingTest, ClearTexture)
{
    RenderTexture texture(*device, 50, 50, vk::Format::eR32Sfloat);

    std::vector<float> data(50*50, 3.5f);

    texture.Record([&](vk::CommandBuffer commandBuffer)
    {
        Clear(50, 50, {3.5f, 0.0f, 0.0f, 0.0f}).Draw(commandBuffer);
    });

    texture.Submit();
    device->Queue().waitIdle();

    Texture outTexture(*device, 50, 50, vk::Format::eR32Sfloat, true);
    device->ExecuteCommand([&](vk::CommandBuffer commandBuffer)
    {
        outTexture.CopyFrom(commandBuffer, texture);
        outTexture.Barrier(commandBuffer, vk::ImageLayout::eGeneral, vk::AccessFlagBits::eHostRead);
    });

    CheckTexture(data, outTexture, 4);
}
