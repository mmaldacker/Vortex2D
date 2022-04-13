//
//  RenderingTests.cpp
//  Vortex
//

#include <gtest/gtest.h>

#include <Vortex/Renderer/CommandBuffer.h>
#include <Vortex/Renderer/RenderTexture.h>
#include <Vortex/Renderer/Shapes.h>
#include <Vortex/Renderer/Sprite.h>
#include <Vortex/Renderer/Texture.h>

#include "ShapeDrawer.h"
#include "Verify.h"

using namespace Vortex::Renderer;

extern Device* device;

TEST(RenderingTest, WriteHostTextureInt)
{
  Texture texture(*device, 50, 50, Format::R8Uint, MemoryUsage::Cpu);

  std::vector<uint8_t> data(50 * 50, 0);
  DrawSquare<uint8_t>(50, 50, data, glm::vec2(10.0f, 15.0f), glm::vec2(5.0f, 8.0f), 12);

  texture.CopyFrom(data);

  CheckTexture(data, texture);
}

TEST(RenderingTest, WriteHostTextureFloat)
{
  Texture texture(*device, 50, 50, Format::R32Sfloat, MemoryUsage::Cpu);

  std::vector<float> data(50 * 50, 0.0f);
  DrawSquare(50, 50, data, glm::vec2(10.0f, 15.0f), glm::vec2(5.0f, 8.0f), 1.0f);

  texture.CopyFrom(data);

  CheckTexture(data, texture);
}

TEST(RenderingTest, TextureCopy)
{
  Texture texture(*device, 50, 50, Format::R8Sint);
  Texture inTexture(*device, 50, 50, Format::R8Sint, MemoryUsage::Cpu);
  Texture outTexture(*device, 50, 50, Format::R8Sint, MemoryUsage::Cpu);

  std::vector<int8_t> data(50 * 50, 0);
  DrawSquare<int8_t>(50, 50, data, glm::vec2(10.0f, 15.0f), glm::vec2(5.0f, 8.0f), -5);

  inTexture.CopyFrom(data);

  device->Execute(
      [&](CommandEncoder& command)
      {
        texture.CopyFrom(command, inTexture);
        outTexture.CopyFrom(command, texture);
      });

  CheckTexture(data, outTexture);
}

TEST(RenderingTest, TextureBufferCopy)
{
  Texture texture(*device, 50, 50, Format::R32Sfloat, MemoryUsage::Cpu);
  Buffer<float> buffer(*device, 50 * 50, MemoryUsage::Cpu);

  std::vector<float> data(50 * 50, 0);
  DrawSquare<float>(50, 50, data, glm::vec2(10.0f, 15.0f), glm::vec2(5.0f, 8.0f), -5.0f);

  texture.CopyFrom(data);

  device->Execute([&](CommandEncoder& command) { buffer.CopyFrom(command, texture); });

  CheckBuffer(data, buffer);
}

TEST(RenderingTest, ClearTexture)
{
  RenderTexture texture(*device, 50, 50, Format::R32Sfloat);

  std::vector<float> data(50 * 50, 3.5f);

  auto clear = std::make_shared<Clear>(glm::vec4{3.5f, 0.0f, 0.0f, 0.0f});

  texture.Record({clear}).Submit();

  Texture outTexture(*device, 50, 50, Format::R32Sfloat, MemoryUsage::Cpu);

  device->Execute([&](CommandEncoder& command) { outTexture.CopyFrom(command, texture); });

  CheckTexture(data, outTexture);
}

TEST(RenderingTest, IntClearTexture)
{
  RenderTexture texture(*device, 50, 50, Format::R32Sint);

  std::vector<int> data(50 * 50, 3);

  device->Execute(
      [&](CommandEncoder& command) {
        texture.Clear(command, std::array<int, 4>{3, 0, 0, 0});
      });

  Texture outTexture(*device, 50, 50, Format::R32Sint, MemoryUsage::Cpu);

  device->Execute([&](CommandEncoder& command) { outTexture.CopyFrom(command, texture); });

  CheckTexture(data, outTexture);
}

TEST(RenderingTest, BlendAdd)
{
  glm::ivec2 size(50);

  RenderTexture texture(*device, size.x, size.y, Format::R32G32Sfloat);

  std::vector<glm::vec2> data(size.x * size.y, glm::vec2(0.1f, 0.0f));
  Texture localTexture(*device, size.x, size.y, Format::R32G32Sfloat, MemoryUsage::Cpu);
  localTexture.CopyFrom(data);

  device->Execute([&](CommandEncoder& command) { texture.CopyFrom(command, localTexture); });

  auto rectangle = std::make_shared<Vortex::Renderer::Rectangle>(*device, size);
  rectangle->Colour = glm::vec4(0.5f, 0.0f, 0.0f, 0.0f);

  Vortex::Renderer::ColorBlendState blendState(Vortex::Renderer::BlendFactor::One,
                                               Vortex::Renderer::BlendFactor::One,
                                               Vortex::Renderer::BlendOp::Add);

  texture.Record({rectangle}, blendState).Submit();
  texture.Record({rectangle}, blendState).Submit();

  device->WaitIdle();

  device->Execute([&](CommandEncoder& command) { localTexture.CopyFrom(command, texture); });

  std::vector<glm::vec2> outData(size.x * size.y, glm::vec2(1.1f, 0.0f));
  CheckTexture<glm::vec2>(outData, localTexture);
}

TEST(RenderingTest, MoveCommandBuffer)
{
  RenderCommand renderCommand;

  RenderTexture texture(*device, 50, 50, Format::R32Sfloat);
  Texture outTexture(*device, 50, 50, Format::R32Sfloat, MemoryUsage::Cpu);

  {
    auto clear1 = std::make_shared<Clear>(glm::vec4{1.0f, 0.0f, 0.0f, 1.0f});

    // clear with 1
    renderCommand = texture.Record({clear1});
    renderCommand.Submit();

    std::vector<float> data1(50 * 50, 1.0f);

    device->Execute([&](CommandEncoder& command) { outTexture.CopyFrom(command, texture); });

    CheckTexture(data1, outTexture);
  }

  {
    auto clear2 = std::make_shared<Clear>(glm::vec4{2.0f, 0.0f, 0.0f, 1.0f});

    // clear with 2
    renderCommand = texture.Record({clear2});
    renderCommand.Submit();

    std::vector<float> data2(50 * 50, 2.0f);
    device->Execute([&](CommandEncoder& command) { outTexture.CopyFrom(command, texture); });

    CheckTexture(data2, outTexture);
  }
}

TEST(RenderingTest, CommandBufferWait)
{
  Buffer<int> buffer(*device, 1, MemoryUsage::Gpu);
  Buffer<int> localBufferRead(*device, 1, MemoryUsage::GpuToCpu);
  Buffer<int> localBufferWrite(*device, 1, MemoryUsage::Cpu);

  CommandBuffer write(*device, false);
  write.Record([&](CommandEncoder& command) { buffer.CopyFrom(command, localBufferWrite); });

  CommandBuffer read(*device, true);
  read.Record([&](CommandEncoder& command) { localBufferRead.CopyFrom(command, buffer); });

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

  Texture texture(*device, size.x, size.y, Format::R8G8B8A8Unorm);
  Texture localTexture(*device, size.x, size.y, Format::R8G8B8A8Unorm, MemoryUsage::Cpu);

  std::vector<glm::u8vec4> data(size.x * size.y, glm::u8vec4(1, 2, 3, 4));
  localTexture.CopyFrom(data);

  device->Execute([&](CommandEncoder& command) { texture.CopyFrom(command, localTexture); });

  RenderTexture output(*device, size.x, size.y, Format::R8G8B8A8Unorm);
  auto sprite = std::make_shared<Sprite>(*device, texture);

  output.Record({sprite}).Submit();
  device->WaitIdle();

  device->Execute([&](CommandEncoder& command) { localTexture.CopyFrom(command, output); });

  CheckTexture<glm::u8vec4>(data, localTexture);
}
