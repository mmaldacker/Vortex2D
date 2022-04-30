//
//  BoundariesTests.cpp
//  Vortex
//

#include "../Renderer/ShapeDrawer.h"
#include "Verify.h"

#include <algorithm>
#include <glm/gtx/io.hpp>

#include <Vortex/Engine/Boundaries.h>
#include <Vortex/Engine/LevelSet.h>
#include <Vortex/Renderer/Shapes.h>

using namespace Vortex::Renderer;
using namespace Vortex::Fluid;

extern Device* device;

void DrawCircle(const glm::ivec2& size,
                std::vector<float>& data,
                float radius,
                const glm::vec2& centre)
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
  if (v >= 0.0)
    return 1.0;
  else
    return -1.0;
}

void DrawSignedSquare(const glm::ivec2& size,
                      const std::vector<glm::vec2>& points,
                      std::vector<float>& data,
                      const glm::vec2& pos)
{
  for (int i = 0; i < size.x; i++)
  {
    for (int j = 0; j < size.y; j++)
    {
      int index = i + j * size.x;
      float value = (float)-std::max(size.x, size.y);
      for (std::size_t k = points.size() - 1, l = 0; l < points.size(); k = l++)
      {
        float udist = DistToSegment(points[k] + pos, points[l] + pos, glm::vec2(i, j));
        float dist = -Orientation(points[k] + pos, points[l] + pos, glm::vec2(i, j)) * udist;
        value = std::max(value, dist);
      }

      data[index] = std::min(data[index], value);
    }
  }
}

void CheckLevelSet(const std::vector<float>& data,
                   Vortex::Renderer::Texture& texture,
                   float error = 1e-5f)
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

  auto square = std::make_shared<Vortex::Fluid::Polygon>(*device, points, false, 20);
  square->Position = glm::vec2(5.0f, 10.0f);

  std::vector<float> data(size.x * size.y, 100.0f);
  DrawSignedSquare(size, points, data, square->Position);

  LevelSet levelSet(*device, size);

  auto clear = std::make_shared<Clear>(glm::vec4{100.0f, 0.0f, 0.0f, 0.0f});

  levelSet.Record({clear, square}).Submit();
  device->WaitIdle();

  Texture outTexture(*device, size.x, size.y, Format::R32Sfloat, MemoryUsage::Cpu);
  device->Execute([&](CommandEncoder& command) { outTexture.CopyFrom(command, levelSet); });

  CheckLevelSet(data, outTexture);
}

TEST(BoundariesTests, InverseSquare)
{
  glm::ivec2 size(20);

  std::vector<glm::vec2> points = {{0.0f, 0.0f}, {4.0f, 0.0f}, {4.0f, 4.0f}, {0.0f, 4.0f}};

  auto square = std::make_shared<Vortex::Fluid::Polygon>(*device, points, true, 20);
  square->Position = glm::vec2(5.0f, 10.0f);

  std::vector<float> data(size.x * size.y, 100.0f);
  DrawSignedSquare(size, points, data, square->Position);

  for (float& x : data)
    x *= -1.0f;

  LevelSet levelSet(*device, size);

  auto clear = std::make_shared<Clear>(glm::vec4{100.0f, 0.0f, 0.0f, 0.0f});

  levelSet.Record({clear, square}).Submit();
  device->WaitIdle();

  Texture outTexture(*device, size.x, size.y, Format::R32Sfloat, MemoryUsage::Cpu);
  device->Execute([&](CommandEncoder& command) { outTexture.CopyFrom(command, levelSet); });

  CheckLevelSet(data, outTexture);
}

TEST(BoundariesTests, Circle)
{
  glm::ivec2 size(20);

  std::vector<glm::vec2> points = {{0.0f, 0.0f}, {4.0f, 0.0f}, {4.0f, 4.0f}, {0.0f, 4.0f}};

  auto circle = std::make_shared<Circle>(*device, 5.0f, 20);
  circle->Position = glm::vec2(8.0f, 10.0f);

  std::vector<float> data(size.x * size.y, 100.0f);
  DrawCircle(size, data, 5.0f, circle->Position);

  LevelSet levelSet(*device, size);
  auto clear = std::make_shared<Clear>(glm::vec4{100.0f, 0.0f, 0.0f, 0.0f});

  levelSet.Record({clear, circle}).Submit();
  device->WaitIdle();

  Texture outTexture(*device, size.x, size.y, Format::R32Sfloat, MemoryUsage::Cpu);
  device->Execute([&](CommandEncoder& command) { outTexture.CopyFrom(command, levelSet); });

  CheckLevelSet(data, outTexture);
}

TEST(BoundariesTests, Intersection)
{
  glm::ivec2 size(20);

  std::vector<glm::vec2> points = {{0.0f, 0.0f}, {4.0f, 0.0f}, {4.0f, 4.0f}, {0.0f, 4.0f}};

  auto square1 = std::make_shared<Vortex::Fluid::Polygon>(*device, points, false, 20);
  square1->Position = glm::vec2(5.0f, 10.0f);

  auto square2 = std::make_shared<Vortex::Fluid::Polygon>(*device, points, false, 20);
  square2->Position = glm::vec2(12.0f, 10.0f);

  std::vector<float> data1(size.x * size.y, 100.0f);
  DrawSignedSquare(size, points, data1, square1->Position);

  std::vector<float> data2(size.x * size.y, 100.0f);
  DrawSignedSquare(size, points, data2, square2->Position);

  std::vector<float> data(size.x * size.y, 100.0f);
  for (std::size_t i = 0; i < data.size(); i++)
  {
    data[i] = std::min(data1[i], data2[i]);
  }

  LevelSet levelSet(*device, size);
  auto clear = std::make_shared<Clear>(glm::vec4{100.0f, 0.0f, 0.0f, 0.0f});

  levelSet.Record({clear, square1, square2}, UnionBlend).Submit();
  device->WaitIdle();

  Texture outTexture(*device, size.x, size.y, Format::R32Sfloat, MemoryUsage::Cpu);
  device->Execute([&](CommandEncoder& command) { outTexture.CopyFrom(command, levelSet); });

  CheckLevelSet(data, outTexture);
}

float clamp(float x, float min, float max)
{
  if (x < min)
    return min;
  if (x > max)
    return max;
  return x;
}

float smoothstep(float edge0, float edge1, float x)
{
  x = clamp((x - edge0) / (edge1 - edge0), 0.0, 1.0);
  return x * x * (3 - 2 * x);
}

TEST(BoundariesTest, DistanceField)
{
  glm::ivec2 size(50);

  LevelSet levelSet(*device, size);

  std::vector<float> data(size.x * size.y, 0.1f);
  Texture localLevelSet(*device, size.x, size.y, Format::R32Sfloat, MemoryUsage::Cpu);
  device->Execute(
      [&](CommandEncoder& command)
      {
        localLevelSet.CopyFrom(data);
        levelSet.CopyFrom(command, localLevelSet);
      });

  auto distance = std::make_shared<DistanceField>(*device, levelSet);
  distance->Colour = glm::vec4(0.2f, 1.0f, 0.8f, 1.0f);

  RenderTexture output(*device, size.x, size.y, Format::R8G8B8A8Unorm);
  Texture localOutput(*device, size.x, size.y, Format::R8G8B8A8Unorm, MemoryUsage::Cpu);

  output.Record({distance}).Submit();
  device->WaitIdle();

  device->Execute([&](CommandEncoder& command) { localOutput.CopyFrom(command, output); });

  auto r = static_cast<uint8_t>(255 * distance->Colour.r);
  auto g = static_cast<uint8_t>(255 * distance->Colour.g);
  auto b = static_cast<uint8_t>(255 * distance->Colour.b);
  auto a = static_cast<uint8_t>(255 - std::round(255 * smoothstep(0.0, 1.0, 0.1f + 0.5f)));
  std::vector<glm::u8vec4> outData(size.x * size.y, {r, g, b, a});

  CheckTexture<glm::u8vec4>(outData, localOutput);
}

TEST(BoundariesTest, Contour)
{
  const glm::ivec2 size(10);
  const int total = size.x * size.y;

  RenderTexture render(*device, size.x, size.y, Format::R32Sfloat);

  auto rectangle = std::make_shared<Vortex::Fluid::Rectangle>(*device, glm::vec2{2, 2});
  rectangle->Position = {2, 2};

  render.Record({rectangle}).Submit().Wait();

  Contour contour(*device, render, size);
  contour.Generate();

  device->WaitIdle();

  IndirectBuffer<DispatchParams> verticesParams(*device, MemoryUsage::Cpu);
  IndirectBuffer<DispatchParams> indicesParams(*device, MemoryUsage::Cpu);

  Buffer<glm::vec2> vertices(*device, total, MemoryUsage::Cpu);
  Buffer<std::uint32_t> indices(*device, total * 4, MemoryUsage::Cpu);

  device->Execute(
      [&](CommandEncoder& command)
      {
        verticesParams.CopyFrom(command, contour.GetVerticesParam());
        indicesParams.CopyFrom(command, contour.GetIndicesParam());

        vertices.CopyFrom(command, contour.GetVertices());
        indices.CopyFrom(command, contour.GetIndices());
      });

  DispatchParams params(0);

  CopyTo(verticesParams, params);

  std::vector<glm::vec2> localVertices(total);
  CopyTo(vertices, localVertices);
  localVertices.resize(params.count);

  auto is_same_vertex = [](auto localVertex, auto vertex)
  {
    auto diff = glm::abs(localVertex - vertex);
    return diff.x < 1e-4f && diff.y < 1e-4f;
  };

  // Check all vertices are present
  auto has_vertex = [&](auto vertex)
  {
    return std::any_of(localVertices.begin(),
                       localVertices.end(),
                       [=](auto localVertex) { return is_same_vertex(localVertex, vertex); });
  };

  glm::vec2 topLeft = glm::vec2{0, 0} + rectangle->Position;
  glm::vec2 topRight = glm::vec2{2, 0} + rectangle->Position;
  glm::vec2 bottomLeft = glm::vec2{0, 2} + rectangle->Position;
  glm::vec2 bottomRight = glm::vec2{2, 2} + rectangle->Position;

  EXPECT_TRUE(has_vertex(topLeft)) << "Missing " << topLeft;
  EXPECT_TRUE(has_vertex(topRight)) << "Missing " << topRight;
  EXPECT_TRUE(has_vertex(bottomLeft)) << "Missing " << bottomLeft;
  EXPECT_TRUE(has_vertex(bottomRight)) << "Missing " << bottomRight;

  EXPECT_EQ(localVertices.size(), 4);

  CopyTo(indicesParams, params);

  std::vector<std::uint32_t> localIndices(total * 4);
  CopyTo(indices, localIndices);
  localIndices.resize(params.count);

  // Check all indices are present
  auto get_vertex_index = [&](auto vertex)
  {
    for (int i = 0; i < localVertices.size(); i++)
    {
      if (is_same_vertex(localVertices[i], vertex))
      {
        return i;
      }
    }

    return -1;
  };

  auto has_index_pair = [&](auto first, auto second)
  {
    for (int i = 1; i < localIndices.size(); i += 2)
    {
      if ((localIndices[i] == first && localIndices[i - 1] == second) ||
          (localIndices[i] == second && localIndices[i - 1] == first))
      {
        return true;
      }
    }

    return false;
  };

  EXPECT_TRUE(has_index_pair(get_vertex_index(topLeft), get_vertex_index(topRight)));
  EXPECT_TRUE(has_index_pair(get_vertex_index(bottomLeft), get_vertex_index(bottomRight)));
  EXPECT_TRUE(has_index_pair(get_vertex_index(topLeft), get_vertex_index(bottomLeft)));
  EXPECT_TRUE(has_index_pair(get_vertex_index(topRight), get_vertex_index(bottomRight)));

  EXPECT_EQ(localIndices.size(), 4 * 2);
}
