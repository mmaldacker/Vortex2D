//
//  RenderingTests.cpp
//  Vortex2D
//

#include <gtest/gtest.h>

#include <Vortex2D/Renderer/Shapes.h>
#include <Vortex2D/Renderer/Sprite.h>
#include <Vortex2D/Renderer/Texture.h>
#include <Vortex2D/Renderer/RenderTexture.h>
#include <Vortex2D/Renderer/CommandBuffer.h>

#include "ShapeDrawer.h"
#include "Verify.h"

using namespace Vortex2D::Renderer;

extern Device* device;

TEST(RenderingTest, WriteHostTextureInt)
{
    Texture texture(*device, 50, 50, vk::Format::eR8Uint, VMA_MEMORY_USAGE_CPU_ONLY);

    std::vector<uint8_t> data(50*50, 0);
    DrawSquare<uint8_t>(50, 50, data, glm::vec2(10.0f, 15.0f), glm::vec2(5.0f, 8.0f), 12);

    texture.CopyFrom(data);

    CheckTexture(data, texture);
}

TEST(RenderingTest, WriteHostTextureFloat)
{
    Texture texture(*device, 50, 50, vk::Format::eR32Sfloat, VMA_MEMORY_USAGE_CPU_ONLY);

    std::vector<float> data(50*50, 0.0f);
    DrawSquare(50, 50, data, glm::vec2(10.0f, 15.0f), glm::vec2(5.0f, 8.0f), 1.0f);

    texture.CopyFrom(data);

    CheckTexture(data, texture);
}

TEST(RenderingTest, TextureCopy)
{
    Texture texture(*device, 50, 50, vk::Format::eR32Sint);
    Texture inTexture(*device, 50, 50, vk::Format::eR32Sint, VMA_MEMORY_USAGE_CPU_ONLY);
    Texture outTexture(*device, 50, 50, vk::Format::eR32Sint, VMA_MEMORY_USAGE_CPU_ONLY);

    std::vector<int8_t> data(50*50, 0);
    DrawSquare<int8_t>(50, 50, data, glm::vec2(10.0f, 15.0f), glm::vec2(5.0f, 8.0f), -5);

    inTexture.CopyFrom(data);

    ExecuteCommand(*device, [&](vk::CommandBuffer commandBuffer)
    {
       texture.CopyFrom(commandBuffer, inTexture);
       outTexture.CopyFrom(commandBuffer, texture);
    });

    CheckTexture(data, outTexture);
}

TEST(RenderingTest, TextureBufferCopy)
{
    Texture texture(*device, 50, 50, vk::Format::eR32Sfloat, VMA_MEMORY_USAGE_CPU_ONLY);
    Buffer<float> buffer(*device, 50*50, VMA_MEMORY_USAGE_CPU_ONLY);

    std::vector<float> data(50*50, 0);
    DrawSquare<float>(50, 50, data, glm::vec2(10.0f, 15.0f), glm::vec2(5.0f, 8.0f), -5.0f);

    texture.CopyFrom(data);

    ExecuteCommand(*device, [&](vk::CommandBuffer commandBuffer)
    {
      buffer.CopyFrom(commandBuffer, texture);
    });

    CheckBuffer(data, buffer);
}

TEST(RenderingTest, ClearTexture)
{
    RenderTexture texture(*device, 50, 50, vk::Format::eR32Sfloat);

    std::vector<float> data(50*50, 3.5f);

    Clear clear({3.5f, 0.0f, 0.0f, 0.0f});

    texture.Record({clear}).Submit();

    Texture outTexture(*device, 50, 50, vk::Format::eR32Sfloat, VMA_MEMORY_USAGE_CPU_ONLY);

    ExecuteCommand(*device, [&](vk::CommandBuffer commandBuffer)
    {
        outTexture.CopyFrom(commandBuffer, texture);
    });

    CheckTexture(data, outTexture);
}

TEST(RenderingTest, IntClearTexture)
{
    RenderTexture texture(*device, 50, 50, vk::Format::eR32Sint);

    std::vector<int> data(50*50, 3);

    ExecuteCommand(*device, [&](vk::CommandBuffer commandBuffer)
    {
        texture.Clear(commandBuffer, std::array<int, 4>{3, 0, 0, 0});
    });

    Texture outTexture(*device, 50, 50, vk::Format::eR32Sint, VMA_MEMORY_USAGE_CPU_ONLY);

    ExecuteCommand(*device, [&](vk::CommandBuffer commandBuffer)
    {
        outTexture.CopyFrom(commandBuffer, texture);
    });

    CheckTexture(data, outTexture);
}

TEST(RenderingTest, BlendAdd)
{
    glm::ivec2 size(50);

    RenderTexture texture(*device, size.x, size.y, vk::Format::eR32G32Sfloat);

    std::vector<glm::vec2> data(size.x*size.y, glm::vec2(0.1f, 0.0f));
    Texture localTexture(*device, size.x, size.y, vk::Format::eR32G32Sfloat, VMA_MEMORY_USAGE_CPU_ONLY);
    localTexture.CopyFrom(data);

    ExecuteCommand(*device, [&](vk::CommandBuffer commandBuffer)
    {
        texture.CopyFrom(commandBuffer, localTexture);
    });

    Rectangle rectangle(*device, size);
    rectangle.Colour = glm::vec4(0.5f, 0.0f, 0.0f, 0.0f);

    Vortex2D::Renderer::ColorBlendState blendState;
    blendState.ColorBlend
            .setBlendEnable(true)
            .setColorBlendOp(vk::BlendOp::eAdd)
            .setSrcColorBlendFactor(vk::BlendFactor::eOne)
            .setDstColorBlendFactor(vk::BlendFactor::eOne);

    texture.Record({rectangle}, blendState).Submit();
    texture.Record({rectangle}, blendState).Submit();

    device->Handle().waitIdle();

    ExecuteCommand(*device, [&](vk::CommandBuffer commandBuffer)
    {
        localTexture.CopyFrom(commandBuffer, texture);
    });

    std::vector<glm::vec2> outData(size.x*size.y, glm::vec2(1.1f, 0.0f));
    CheckTexture<glm::vec2>(outData, localTexture);
}

TEST(RenderingTest, MoveCommandBuffer)
{
    RenderCommand renderCommand;

    RenderTexture texture(*device, 50, 50, vk::Format::eR32Sfloat);
    Texture outTexture(*device, 50, 50, vk::Format::eR32Sfloat, VMA_MEMORY_USAGE_CPU_ONLY);

    {
        Clear clear1({1.0f, 0.0f, 0.0f, 1.0f});

        // clear with 1
        renderCommand = texture.Record({clear1});
        renderCommand.Submit();

        std::vector<float> data1(50*50, 1.0f);

        ExecuteCommand(*device, [&](vk::CommandBuffer commandBuffer)
        {
            outTexture.CopyFrom(commandBuffer, texture);
        });

        CheckTexture(data1, outTexture);
    }

    {
        Clear clear2({2.0f, 0.0f, 0.0f, 1.0f});

        // clear with 2
        renderCommand = texture.Record({clear2});
        renderCommand.Submit();

        std::vector<float> data2(50*50, 2.0f);
        ExecuteCommand(*device, [&](vk::CommandBuffer commandBuffer)
        {
            outTexture.CopyFrom(commandBuffer, texture);
        });

        CheckTexture(data2, outTexture);
    }
}

TEST(RenderingTest, CommandBufferWait)
{
  Buffer<int> buffer(*device, 1, VMA_MEMORY_USAGE_GPU_ONLY);
  Buffer<int> localBufferRead(*device, 1, VMA_MEMORY_USAGE_GPU_TO_CPU);
  Buffer<int> localBufferWrite(*device, 1, VMA_MEMORY_USAGE_CPU_ONLY);

  CommandBuffer write(*device, false);
  write.Record([&](vk::CommandBuffer commandBuffer)
  {
      buffer.CopyFrom(commandBuffer, localBufferWrite);
  });

  CommandBuffer read(*device, true);
  read.Record([&](vk::CommandBuffer commandBuffer)
  {
      localBufferRead.CopyFrom(commandBuffer, buffer);
  });

  for (int i = 0; i < 10; i++)
  {
    CopyFrom(localBufferWrite, i);
    write.Submit();
    read.Submit().Wait();

    int result;
    CopyTo(localBufferRead, result);
    ASSERT_EQ(i, result);
  }

}

TEST(RenderingTest, Sprite)
{
    glm::ivec2 size(20);

    Texture texture(*device, size.x, size.y, vk::Format::eR8G8B8A8Unorm);
    Texture localTexture(*device, size.x, size.y, vk::Format::eR8G8B8A8Unorm, VMA_MEMORY_USAGE_CPU_ONLY);

    std::vector<glm::u8vec4> data(size.x*size.y, glm::u8vec4(1, 2, 3, 4));
    localTexture.CopyFrom(data);

    ExecuteCommand(*device, [&](vk::CommandBuffer commandBuffer)
    {
        texture.CopyFrom(commandBuffer, localTexture);
    });

    RenderTexture output(*device, size.x, size.y, vk::Format::eR8G8B8A8Unorm);
    Sprite sprite(*device, texture);

    output.Record({sprite}).Submit();
    device->Handle().waitIdle();

    ExecuteCommand(*device, [&](vk::CommandBuffer commandBuffer)
    {
        localTexture.CopyFrom(commandBuffer, output);
    });

    CheckTexture<glm::u8vec4>(data, localTexture);
}
