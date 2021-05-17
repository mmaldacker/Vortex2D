//
//  Boundaries.cpp
//  Vortex
//

#include "Boundaries.h"

#include <Vortex/Engine/LevelSet.h>
#include <Vortex/Engine/Particles.h>
#include <Vortex/SPIRV/Reflection.h>

#include "vortex_generated_spirv.h"

namespace Vortex
{
namespace Fluid
{
namespace
{
bool IsClockwise(const std::vector<glm::vec2>& points)
{
  float total = 0.0f;
  for (std::size_t i = points.size() - 1, j = 0; j < points.size(); i = j++)
  {
    total += (points[j].x - points[i].x) * (points[i].y + points[j].y);
  }

  return total > 0.0;
}

std::vector<glm::vec2> GetBoundingBox(const std::vector<glm::vec2>& points, float extent)
{
  glm::vec2 topLeft(std::numeric_limits<float>::max());
  glm::vec2 bottomRight(std::numeric_limits<float>::min());

  for (auto& point : points)
  {
    topLeft.x = glm::min(topLeft.x, point.x);
    topLeft.y = glm::min(topLeft.y, point.y);

    bottomRight.x = glm::max(bottomRight.x, point.x);
    bottomRight.y = glm::max(bottomRight.y, point.y);
  }

  topLeft -= glm::vec2(extent);
  bottomRight += glm::vec2(extent);

  return {{topLeft.x, topLeft.y},
          {bottomRight.x, topLeft.y},
          {topLeft.x, bottomRight.y},
          {
              bottomRight.x,
              topLeft.y,
          },
          {bottomRight.x, bottomRight.y},
          {topLeft.x, bottomRight.y}};
}

}  // namespace

Polygon::Polygon(const Renderer::Device& device,
                 std::vector<glm::vec2> points,
                 bool inverse,
                 float extent)
    : mDevice(device)
    , mSize(static_cast<uint32_t>(points.size()))
    , mInv(inverse)
    , mMVPBuffer(device, VMA_MEMORY_USAGE_CPU_TO_GPU)
    , mMVBuffer(device, VMA_MEMORY_USAGE_CPU_TO_GPU)
    , mVertexBuffer(device, 6ul)
    , mPolygonVertexBuffer(device, static_cast<unsigned>(points.size()))
{
  assert(!IsClockwise(points));
  if (inverse)
  {
    std::reverse(points.begin(), points.end());
  }

  Renderer::Buffer<glm::vec2> localPolygonVertexBuffer(
      device, points.size(), VMA_MEMORY_USAGE_CPU_ONLY);
  Renderer::CopyFrom(localPolygonVertexBuffer, points);

  auto boundingBox = GetBoundingBox(points, extent);
  Renderer::Buffer<glm::vec2> localVertexBuffer(
      device, boundingBox.size(), VMA_MEMORY_USAGE_CPU_ONLY);
  Renderer::CopyFrom(localVertexBuffer, boundingBox);

  device.Execute([&](vk::CommandBuffer commandBuffer) {
    mPolygonVertexBuffer.CopyFrom(commandBuffer, localPolygonVertexBuffer);
    mVertexBuffer.CopyFrom(commandBuffer, localVertexBuffer);
  });

  SPIRV::Reflection reflectionVert(SPIRV::Position_vert);
  SPIRV::Reflection reflectionFrag(SPIRV::PolygonDist_frag);

  Renderer::PipelineLayout layout = {{reflectionVert, reflectionFrag}};
  mDescriptorSet = device.GetLayoutManager().MakeDescriptorSet(layout);
  Bind(device, mDescriptorSet, layout, {{mMVPBuffer}, {mMVBuffer}, {mPolygonVertexBuffer}});

  mPipeline =
      Renderer::GraphicsPipeline()
          .Topology(vk::PrimitiveTopology::eTriangleList)
          .Shader(device.GetShaderModule(SPIRV::Position_vert), vk::ShaderStageFlagBits::eVertex)
          .Shader(device.GetShaderModule(SPIRV::PolygonDist_frag),
                  vk::ShaderStageFlagBits::eFragment)
          .VertexAttribute(0, 0, vk::Format::eR32G32Sfloat, 0)
          .VertexBinding(0, sizeof(glm::vec2))
          .Layout(mDescriptorSet.pipelineLayout);
}

Polygon::~Polygon() {}

void Polygon::Initialize(const Renderer::RenderState& renderState)
{
  auto pipeline = mDevice.GetPipelineCache().CreateGraphicsPipeline(mPipeline, renderState);
}

void Polygon::Update(const glm::mat4& projection, const glm::mat4& view)
{
  Transformable::Update();
  Renderer::CopyFrom(mMVPBuffer, projection * view * GetTransform());
  Renderer::CopyFrom(mMVBuffer, view * GetTransform());
}

void Polygon::Draw(vk::CommandBuffer commandBuffer, const Renderer::RenderState& renderState)
{
  auto pipeline = mDevice.GetPipelineCache().CreateGraphicsPipeline(mPipeline, renderState);
  commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline);
  commandBuffer.pushConstants(
      mDescriptorSet.pipelineLayout, vk::ShaderStageFlagBits::eFragment, 0, 4, &mSize);
  commandBuffer.pushConstants(
      mDescriptorSet.pipelineLayout, vk::ShaderStageFlagBits::eFragment, 4, 4, &mInv);
  commandBuffer.bindVertexBuffers(0, {mVertexBuffer.Handle()}, {0ul});
  commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics,
                                   mDescriptorSet.pipelineLayout,
                                   0,
                                   {*mDescriptorSet.descriptorSet},
                                   {});
  commandBuffer.draw(6, 1, 0, 0);
}

Rectangle::Rectangle(const Renderer::Device& device,
                     const glm::vec2& size,
                     bool inverse,
                     float extent)
    : Polygon(device,
              {{0.0f, 0.0f}, {size.x, 0.0f}, {size.x, size.y}, {0.0f, size.y}},
              inverse,
              extent)
{
}

Rectangle::~Rectangle() {}

void Rectangle::Initialize(const Renderer::RenderState& renderState)
{
  Polygon::Initialize(renderState);
}

void Rectangle::Update(const glm::mat4& projection, const glm::mat4& view)
{
  Polygon::Update(projection, view);
}

void Rectangle::Draw(vk::CommandBuffer commandBuffer, const Renderer::RenderState& renderState)
{
  Polygon::Draw(commandBuffer, renderState);
}

Circle::Circle(const Renderer::Device& device, float radius, float extent)
    : mDevice(device)
    , mSize(radius)
    , mMVPBuffer(device, VMA_MEMORY_USAGE_CPU_TO_GPU)
    , mMVBuffer(device, VMA_MEMORY_USAGE_CPU_TO_GPU)
    , mVertexBuffer(device, 6)
{
  std::vector<glm::vec2> points = {
      {-radius, -radius}, {radius, -radius}, {radius, radius}, {-radius, radius}};

  auto boundingBox = GetBoundingBox(points, extent);
  Renderer::Buffer<glm::vec2> localVertexBuffer(
      device, boundingBox.size(), VMA_MEMORY_USAGE_CPU_ONLY);
  Renderer::CopyFrom(localVertexBuffer, boundingBox);

  device.Execute([&](vk::CommandBuffer commandBuffer) {
    mVertexBuffer.CopyFrom(commandBuffer, localVertexBuffer);
  });

  SPIRV::Reflection reflectionVert(SPIRV::Position_vert);
  SPIRV::Reflection reflectionFrag(SPIRV::CircleDist_frag);

  Renderer::PipelineLayout layout = {{reflectionVert, reflectionFrag}};
  mDescriptorSet = device.GetLayoutManager().MakeDescriptorSet(layout);
  Bind(device, mDescriptorSet, layout, {{mMVPBuffer}, {mMVBuffer}});

  mPipeline =
      Renderer::GraphicsPipeline()
          .Topology(vk::PrimitiveTopology::eTriangleList)
          .Shader(device.GetShaderModule(SPIRV::Position_vert), vk::ShaderStageFlagBits::eVertex)
          .Shader(device.GetShaderModule(SPIRV::CircleDist_frag),
                  vk::ShaderStageFlagBits::eFragment)
          .VertexAttribute(0, 0, vk::Format::eR32G32Sfloat, 0)
          .VertexBinding(0, sizeof(glm::vec2))
          .Layout(mDescriptorSet.pipelineLayout);
}

Circle::~Circle() {}

void Circle::Initialize(const Renderer::RenderState& renderState)
{
  mDevice.GetPipelineCache().CreateGraphicsPipeline(mPipeline, renderState);
}

void Circle::Update(const glm::mat4& projection, const glm::mat4& view)
{
  Transformable::Update();
  Renderer::CopyFrom(mMVPBuffer, projection * view * GetTransform());
  Renderer::CopyFrom(mMVBuffer, view * GetTransform());
}

void Circle::Draw(vk::CommandBuffer commandBuffer, const Renderer::RenderState& renderState)
{
  auto pipeline = mDevice.GetPipelineCache().CreateGraphicsPipeline(mPipeline, renderState);
  commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline);
  commandBuffer.pushConstants(
      mDescriptorSet.pipelineLayout, vk::ShaderStageFlagBits::eFragment, 0, 4, &mSize);
  commandBuffer.bindVertexBuffers(0, {mVertexBuffer.Handle()}, {0ul});
  commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics,
                                   mDescriptorSet.pipelineLayout,
                                   0,
                                   {*mDescriptorSet.descriptorSet},
                                   {});
  commandBuffer.draw(6, 1, 0, 0);
}

Renderer::ColorBlendState IntersectionBlend = [] {
  Renderer::ColorBlendState blendState;
  blendState.ColorBlend.setBlendEnable(true)
      .setColorBlendOp(vk::BlendOp::eMax)
      .setSrcColorBlendFactor(vk::BlendFactor::eOne)
      .setDstColorBlendFactor(vk::BlendFactor::eOne)
      .setColorWriteMask(vk::ColorComponentFlagBits::eR);

  return blendState;
}();

Renderer::ColorBlendState UnionBlend = [] {
  Renderer::ColorBlendState blendState;
  blendState.ColorBlend.setBlendEnable(true)
      .setColorBlendOp(vk::BlendOp::eMin)
      .setSrcColorBlendFactor(vk::BlendFactor::eOne)
      .setDstColorBlendFactor(vk::BlendFactor::eOne)
      .setColorWriteMask(vk::ColorComponentFlagBits::eR);

  return blendState;
}();

Vortex::Renderer::Clear BoundariesClear = Vortex::Renderer::Clear({10000.0f, 0.0f, 0.0f, 0.0f});

DistanceField::DistanceField(const Renderer::Device& device,
                             Renderer::RenderTexture& levelSet,
                             float scale)
    : Renderer::AbstractSprite(device, SPIRV::DistanceField_frag, levelSet), mScale(scale)
{
}

DistanceField::DistanceField(DistanceField&& other)
    : Renderer::AbstractSprite(std::move(other)), mScale(other.mScale)
{
}

DistanceField::~DistanceField() {}

void DistanceField::Draw(vk::CommandBuffer commandBuffer, const Renderer::RenderState& renderState)
{
  PushConstant(commandBuffer, 0, mScale);
  AbstractSprite::Draw(commandBuffer, renderState);
}

}  // namespace Fluid
}  // namespace Vortex
