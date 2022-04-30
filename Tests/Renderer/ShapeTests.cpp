//
//  ShapeTests.cpp
//  Vortex
//

#include "ShapeDrawer.h"
#include "Verify.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/transform.hpp>

#include <Vortex/Renderer/CommandBuffer.h>
#include <Vortex/Renderer/RenderState.h>
#include <Vortex/Renderer/RenderTexture.h>
#include <Vortex/Renderer/Shapes.h>
#include <gtest/gtest.h>

using namespace Vortex::Renderer;

extern Device* device;

TEST(ShapeTests, Square)
{
  glm::vec2 size = {10.0f, 20.0f};

  auto rect = std::make_shared<Vortex::Renderer::Rectangle>(*device, size);
  rect->Position = glm::vec2(5.0f, 7.0f);
  rect->Scale = glm::vec2(1.5f, 1.0f);
  rect->Colour = glm::vec4(1.0f);

  auto clear = std::make_shared<Clear>(glm::vec4(0.0f));

  RenderTexture texture(*device, 50, 50, Format::R32Sfloat);
  Texture outTexture(*device, 50, 50, Format::R32Sfloat, MemoryUsage::Cpu);

  texture.Record({clear, rect}).Submit();

  size *= rect->Scale;

  std::vector<float> data(50 * 50, 0.0f);
  DrawSquare(50, 50, data, rect->Position, size, 1.0f);

  device->Execute([&](CommandEncoder& command) { outTexture.CopyFrom(command, texture); });

  CheckTexture(data, outTexture);
}

TEST(ShapeTests, Mesh)
{
  glm::vec2 size = {10.0f, 20.0f};

  std::vector<glm::vec2> vertices = {
      {0.0f, 0.0f}, {size.x, 0.0f}, {0.0f, size.y}, {size.x, size.y}};

  VertexBuffer<glm::vec2> localVertices(*device, vertices.size(), MemoryUsage::Cpu);
  CopyFrom(localVertices, vertices);

  VertexBuffer<glm::vec2> vertexBuffer(*device, vertices.size());
  device->Execute([&](CommandEncoder& command) { vertexBuffer.CopyFrom(command, localVertices); });

  std::vector<std::uint32_t> indices = {0, 1, 1, 3, 3, 2, 2, 0};

  VertexBuffer<std::uint32_t> localIndices(*device, indices.size(), MemoryUsage::Cpu);
  CopyFrom(localIndices, indices);

  IndexBuffer<std::uint32_t> indexBuffer(*device, indices.size());
  device->Execute([&](CommandEncoder& command) { indexBuffer.CopyFrom(command, localIndices); });

  Buffer<DrawIndexedIndirect> parameters(*device, 1, MemoryUsage::Cpu);

  DrawIndexedIndirect localParameters;
  localParameters.indexCount = indices.size();
  localParameters.instanceCount = 1;

  CopyFrom(parameters, localParameters);

  auto mesh = std::make_shared<Mesh>(*device, vertexBuffer, indexBuffer, parameters);
  mesh->Position = glm::vec2(5.0f, 7.0f);
  mesh->Colour = glm::vec4(1.0f);

  auto clear = std::make_shared<Clear>(glm::vec4(0.0f));

  RenderTexture texture(*device, 50, 50, Format::R32Sfloat);
  Texture outTexture(*device, 50, 50, Format::R32Sfloat, MemoryUsage::Cpu);

  texture.Record({clear, mesh}).Submit();

  std::vector<float> data(50 * 50, 0.0f);

  // TODO first pixel isn't drawn, why?
  DrawSquareContour(50, 50, data, mesh->Position - glm::vec2(1.0f), size, 1.0f);

  device->Execute([&](CommandEncoder& command) { outTexture.CopyFrom(command, texture); });

  CheckTexture(data, outTexture);
}

TEST(ShapeTests, IntSquare)
{
  glm::vec2 size = {10.0f, 20.0f};

  auto rect = std::make_shared<IntRectangle>(*device, size);
  rect->Position = glm::vec2(5.0f, 7.0f);
  rect->Colour = glm::vec4(1);

  RenderTexture texture(*device, 50, 50, Format::R32Sint);
  Texture outTexture(*device, 50, 50, Format::R32Sint, MemoryUsage::Cpu);

  texture.Record({rect}).Submit();

  std::vector<int> data(50 * 50, 0);
  DrawSquare(50, 50, data, rect->Position, size, 1);

  device->Execute([&](CommandEncoder& command) { outTexture.CopyFrom(command, texture); });

  CheckTexture(data, outTexture);
}

TEST(ShapeTests, MultipleSquares)
{
  glm::vec2 size = {10.0f, 20.0f};

  auto rect1 = std::make_shared<Vortex::Renderer::Rectangle>(*device, size);
  rect1->Position = glm::vec2(5.0f, 7.0f);
  rect1->Scale = glm::vec2(1.5f, 1.0f);
  rect1->Colour = glm::vec4(1.0f);

  auto rect2 = std::make_shared<Vortex::Renderer::Rectangle>(*device, size);
  rect2->Position = glm::vec2(20.0f, 27.0f);
  rect2->Colour = glm::vec4(1.0f);

  auto clear = std::make_shared<Clear>(glm::vec4(0.0f));

  RenderTexture texture(*device, 50, 50, Format::R32Sfloat);
  Texture outTexture(*device, 50, 50, Format::R32Sfloat, MemoryUsage::Cpu);

  texture.Record({clear, rect1}).Submit();
  texture.Record({rect2}).Submit();

  std::vector<float> data(50 * 50, 0.0f);
  DrawSquare(50, 50, data, rect2->Position, size, 1.0f);

  size *= rect1->Scale;
  DrawSquare(50, 50, data, rect1->Position, size, 1.0f);

  device->Execute([&](CommandEncoder& command) { outTexture.CopyFrom(command, texture); });

  CheckTexture(data, outTexture);
}

TEST(ShapeTests, Circle)
{
  auto ellipse = std::make_shared<Vortex::Renderer::Ellipse>(*device, glm::vec2(5.0f));
  ellipse->Position = glm::vec2(10.0f, 15.0f);
  ellipse->Colour = glm::vec4(1.0f);

  auto clear = std::make_shared<Clear>(glm::vec4(0.0f));

  RenderTexture texture(*device, 50, 50, Format::R32Sfloat);
  Texture outTexture(*device, 50, 50, Format::R32Sfloat, MemoryUsage::Cpu);

  texture.Record({clear, ellipse}).Submit();

  std::vector<float> data(50 * 50, 0.0f);
  DrawCircle(50, 50, data, ellipse->Position, 5.0f);

  device->Execute([&](CommandEncoder& command) { outTexture.CopyFrom(command, texture); });

  CheckTexture(data, outTexture);
}

TEST(ShapeTests, Ellipse)
{
  glm::vec2 radius(4.0f, 7.0f);

  auto ellipse = std::make_shared<Vortex::Renderer::Ellipse>(*device, radius);
  ellipse->Position = glm::vec2(20.0f, 15.0f);
  ellipse->Colour = glm::vec4(1.0f);

  auto clear = std::make_shared<Clear>(glm::vec4(0.0f));

  RenderTexture texture(*device, 50, 50, Format::R32Sfloat);
  Texture outTexture(*device, 50, 50, Format::R32Sfloat, MemoryUsage::Cpu);

  texture.Record({clear, ellipse}).Submit();

  std::vector<float> data(50 * 50, 0.0f);
  DrawEllipse(50, 50, data, ellipse->Position, radius);

  device->Execute([&](CommandEncoder& command) { outTexture.CopyFrom(command, texture); });

  CheckTexture(data, outTexture);
}

TEST(ShapeTests, ScaledEllipse)
{
  glm::vec2 pos(20.0f, 15.0f);
  glm::vec2 radius(4.0f, 7.0f);

  auto ellipse = std::make_shared<Vortex::Renderer::Ellipse>(*device, radius);
  ellipse->Position = pos;
  ellipse->Scale = glm::vec2(1.0f, 2.0f);
  ellipse->Colour = glm::vec4(1.0f);

  auto clear = std::make_shared<Clear>(glm::vec4(0.0f));

  RenderTexture texture(*device, 50, 50, Format::R32Sfloat);
  Texture outTexture(*device, 50, 50, Format::R32Sfloat, MemoryUsage::Cpu);

  texture.Record({clear, ellipse}).Submit();

  radius *= ellipse->Scale;
  std::vector<float> data(50 * 50, 0.0f);
  DrawEllipse(50, 50, data, ellipse->Position, radius);

  device->Execute([&](CommandEncoder& command) { outTexture.CopyFrom(command, texture); });

  CheckTexture(data, outTexture);
}

TEST(ShapeTests, RotatedEllipse)
{
  glm::vec2 radius(4.0f, 7.0f);

  auto ellipse = std::make_shared<Vortex::Renderer::Ellipse>(*device, radius);
  ellipse->Position = glm::vec2(20.0f, 15.0f);
  ellipse->Rotation = 33.0f;
  ellipse->Colour = glm::vec4(1.0f);

  auto clear = std::make_shared<Clear>(glm::vec4(0.0f));

  RenderTexture texture(*device, 50, 50, Format::R32Sfloat);
  Texture outTexture(*device, 50, 50, Format::R32Sfloat, MemoryUsage::Cpu);

  texture.Record({clear, ellipse}).Submit();

  std::vector<float> data(50 * 50, 0.0f);
  DrawEllipse(50, 50, data, ellipse->Position, radius, ellipse->Rotation);

  device->Execute([&](CommandEncoder& command) { outTexture.CopyFrom(command, texture); });

  CheckTexture(data, outTexture);
}

TEST(ShapeTests, RenderScaledEllipse)
{
  glm::vec2 pos(10.0f, 10.0f);
  glm::vec2 radius(5.0f, 8.0f);

  auto ellipse = std::make_shared<Vortex::Renderer::Ellipse>(*device, radius);
  ellipse->Position = pos;
  ellipse->Colour = glm::vec4(1.0f);

  auto clear = std::make_shared<Clear>(glm::vec4(0.0f));

  RenderTexture texture(*device, 50, 50, Format::R32Sfloat);
  Texture outTexture(*device, 50, 50, Format::R32Sfloat, MemoryUsage::Cpu);

  texture.SetView(glm::scale(glm::vec3(2.0f, 2.0f, 1.0f)));
  texture.Record({clear, ellipse}).Submit();

  radius *= glm::vec2(2.0f);
  pos *= glm::vec2(2.0f);
  std::vector<float> data(50 * 50, 0.0f);
  DrawEllipse(50, 50, data, pos, radius);

  device->Execute([&](CommandEncoder& command) { outTexture.CopyFrom(command, texture); });

  CheckTexture(data, outTexture);
}
