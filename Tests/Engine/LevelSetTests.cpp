//
//  LevelSetSet.h
//  Vortex
//

#include "Renderer/ShapeDrawer.h"
#include "VariationalHelpers.h"
#include "Verify.h"

#include <Vortex/Engine/LevelSet.h>
#include <Vortex/Renderer/Shapes.h>

using namespace Vortex::Renderer;
using namespace Vortex::Fluid;

extern Device* device;

void PrintLevelSet(int size, float (*phi)(const Vec2f&))
{
  for (int j = 0; j < size; j++)
  {
    for (int i = 0; i < size; i++)
    {
      Vec2f pos((i + 1.0f) / size, (j + 1.0f) / size);
      std::cout << "(" << size * phi(pos) << ")";
    }
    std::cout << std::endl;
  }
  std::cout << std::endl;
}

void CheckDifference(Texture& texture,
                     float (*phi)(const Vec2f&),
                     float error = 1.0f / std::sqrt(2.0f))
{
  std::vector<float> pixels(texture.GetWidth() * texture.GetHeight());
  texture.CopyTo(pixels);

  for (uint32_t j = 0; j < texture.GetHeight(); j++)
  {
    for (uint32_t i = 0; i < texture.GetWidth(); i++)
    {
      float size = (float)texture.GetWidth();
      Vec2f pos((i + 1.0f) / size, (j + 1.0f) / size);
      float value = texture.GetWidth() * phi(pos);

      float readerValue = pixels[i + j * texture.GetWidth()];
      float diff = std::abs(value - readerValue);

      EXPECT_LT(diff, error) << "Mismatch at " << i << ", " << j;  // almost sqrt(0.5)
    }
  }
}

TEST(LevelSetTests, SimpleCircle)
{
  glm::ivec2 size(50);

  LevelSet levelSet(*device, size, 2000);
  Texture outTexture(*device, size.x, size.y, vk::Format::eR32Sfloat, VMA_MEMORY_USAGE_CPU_ONLY);

  Ellipse circle(*device, glm::vec2{rad0} * glm::vec2(size));
  circle.Position = glm::vec2(c0[0], c0[1]) * glm::vec2(size) - glm::vec2(0.5f);
  circle.Colour = glm::vec4(0.5f);

  Clear clear(glm::vec4(-0.5f));

  levelSet.Record({clear, circle}).Submit();
  levelSet.Reinitialise();

  device->Handle().waitIdle();

  device->Execute(
      [&](vk::CommandBuffer commandBuffer) { outTexture.CopyFrom(commandBuffer, levelSet); });

  // NOTE difference should be at most 1 / sqrt(2)
  CheckDifference(outTexture, boundary_phi, 1.0f);
}

TEST(LevelSetTests, ComplexCircles)
{
  glm::ivec2 size(50);

  LevelSet levelSet(*device, size, 2000);
  Texture outTexture(*device, size.x, size.y, vk::Format::eR32Sfloat, VMA_MEMORY_USAGE_CPU_ONLY);

  Ellipse circle0(*device, glm::vec2{rad0} * glm::vec2(size));
  Ellipse circle1(*device, glm::vec2{rad1} * glm::vec2(size));
  Ellipse circle2(*device, glm::vec2{rad2} * glm::vec2(size));
  Ellipse circle3(*device, glm::vec2{rad3} * glm::vec2(size));

  Clear clear(glm::vec4(-1.0f));

  circle0.Position = glm::vec2(c0[0], c0[1]) * glm::vec2(size) - glm::vec2(0.5f);
  circle1.Position = glm::vec2(c1[0], c1[1]) * glm::vec2(size) - glm::vec2(0.5f);
  circle2.Position = glm::vec2(c2[0], c2[1]) * glm::vec2(size) - glm::vec2(0.5f);
  circle3.Position = glm::vec2(c3[0], c3[1]) * glm::vec2(size) - glm::vec2(0.5f);

  circle0.Colour = glm::vec4(1.0f);
  circle1.Colour = glm::vec4(-1.0f);
  circle2.Colour = glm::vec4(-1.0f);
  circle3.Colour = glm::vec4(-1.0f);

  levelSet.Record({clear, circle0, circle1, circle2, circle3}).Submit();
  levelSet.Reinitialise();

  device->Handle().waitIdle();

  device->Execute(
      [&](vk::CommandBuffer commandBuffer) { outTexture.CopyFrom(commandBuffer, levelSet); });

  // NOTE difference should be at most 1 / sqrt(2)
  CheckDifference(outTexture, complex_boundary_phi, 1.0f);
}

TEST(LevelSetTests, Extrapolate)
{
  glm::ivec2 size(50);

  Texture localSolidPhi(*device, size.x, size.y, vk::Format::eR32Sfloat, VMA_MEMORY_USAGE_CPU_ONLY);
  Texture localLiquidPhi(
      *device, size.x, size.y, vk::Format::eR32Sfloat, VMA_MEMORY_USAGE_CPU_ONLY);
  Texture solidPhi(*device, size.x, size.y, vk::Format::eR32Sfloat);

  std::vector<float> solidData(size.x * size.y, 1.0);
  DrawSquare(size.x, size.y, solidData, glm::vec2(10.0f), glm::vec2(20.0f), -1.0f);
  localSolidPhi.CopyFrom(solidData);

  device->Execute(
      [&](vk::CommandBuffer commandBuffer) { solidPhi.CopyFrom(commandBuffer, localSolidPhi); });

  LevelSet liquidPhi(*device, size);

  liquidPhi.ExtrapolateBind(solidPhi);

  liquidPhi.Extrapolate();

  device->Handle().waitIdle();

  device->Execute(
      [&](vk::CommandBuffer commandBuffer) { localLiquidPhi.CopyFrom(commandBuffer, liquidPhi); });

  std::vector<float> liquidData(size.x * size.y);
  localLiquidPhi.CopyTo(liquidData);

  for (int i = 0; i < 9; i++)
  {
    EXPECT_FLOAT_EQ(-0.5f, liquidData[i + 10 + 10 * size.x]);
    EXPECT_FLOAT_EQ(-0.5f, liquidData[10 + (i + 10) * size.x]);
    EXPECT_FLOAT_EQ(-0.5f, liquidData[i + 10 + 20 * size.x]);
    EXPECT_FLOAT_EQ(-0.5f, liquidData[20 + (i + 10) * size.x]);
  }
}

TEST(LevelSetTests, ShrinkWrap)
{
  glm::ivec2 size(10);
  LevelSet levelSet(*device, size);

  Texture inTexture(*device, size.x, size.y, vk::Format::eR32Sfloat, VMA_MEMORY_USAGE_CPU_ONLY);
  Texture outTexture(*device, size.x, size.y, vk::Format::eR32Sfloat, VMA_MEMORY_USAGE_CPU_ONLY);

  std::vector<float> inData(size.x * size.y, -0.5f);
  inData[6 + 5 * 5] = -0.4f;

  inTexture.CopyFrom(inData);

  device->Execute(
      [&](vk::CommandBuffer commandBuffer) { levelSet.CopyFrom(commandBuffer, inTexture); });

  levelSet.ShrinkWrap();

  device->Execute(
      [&](vk::CommandBuffer commandBuffer) { outTexture.CopyFrom(commandBuffer, levelSet); });

  std::vector<float> outData(size.x * size.y, -0.5f);
  CheckTexture(outData, outTexture);
}
