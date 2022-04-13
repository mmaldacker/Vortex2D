//
//  Shapes.cpp
//  Vortex
//

#include "Shapes.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/transform.hpp>

#include <Vortex/Renderer/CommandBuffer.h>
#include <Vortex/Renderer/RenderTarget.h>
#include <Vortex/SPIRV/Reflection.h>

#include "vortex_generated_spirv.h"

namespace Vortex
{
namespace Renderer
{
AbstractShape::AbstractShape(Device& device,
                             const SpirvBinary& fragName,
                             const std::vector<glm::vec2>& vertices)
    : mDevice(device)
    , mMVPBuffer(device, MemoryUsage::CpuToGpu)
    , mColourBuffer(device, MemoryUsage::CpuToGpu)
    , mVertexBuffer(device, vertices.size())
    , mNumVertices(static_cast<uint32_t>(vertices.size()))
{
  VertexBuffer<glm::vec2> localVertices(device, vertices.size(), MemoryUsage::Cpu);
  Renderer::CopyFrom(localVertices, vertices);
  device.Execute([&](CommandEncoder& command) { mVertexBuffer.CopyFrom(command, localVertices); });

  SPIRV::Reflection reflectionVert(SPIRV::Position_vert);
  SPIRV::Reflection reflectionFrag(fragName);

  SPIRV::ShaderLayouts layout = {reflectionVert, reflectionFrag};

  mPipelineLayout = mDevice.CreatePipelineLayout(layout);
  auto bindGroupLayout = mDevice.CreateBindGroupLayout(layout);
  mBindGroup =
      mDevice.CreateBindGroup(bindGroupLayout, layout, {{mMVPBuffer, 0}, {mColourBuffer, 1}});

  Handle::ShaderModule vertexShader = mDevice.CreateShaderModule(SPIRV::Position_vert);
  Handle::ShaderModule fragShader = mDevice.CreateShaderModule(fragName);

  mPipeline = GraphicsPipelineDescriptor()
                  .Shader(vertexShader, ShaderStage::Vertex)
                  .Shader(fragShader, ShaderStage::Fragment)
                  .VertexAttribute(0, 0, Format::R32G32Sfloat, 0)
                  .VertexBinding(0, sizeof(glm::vec2))
                  .Layout(mPipelineLayout);
}

AbstractShape::AbstractShape(AbstractShape&& other)
    : mDevice(other.mDevice)
    , mMVPBuffer(std::move(other.mMVPBuffer))
    , mColourBuffer(std::move(other.mColourBuffer))
    , mVertexBuffer(std::move(other.mVertexBuffer))
    , mPipelineLayout(std::move(other.mPipelineLayout))
    , mBindGroup(std::move(other.mBindGroup))
    , mPipeline(std::move(other.mPipeline))
    , mNumVertices(other.mNumVertices)
{
  other.mNumVertices = 0;
}

AbstractShape::~AbstractShape() {}

void AbstractShape::Initialize(const RenderState& renderState)
{
  auto pipeline = mDevice.CreateGraphicsPipeline(mPipeline, renderState);
}

void AbstractShape::Update(const glm::mat4& projection, const glm::mat4& view)
{
  Transformable::Update();
  Renderer::CopyFrom(mMVPBuffer, projection * view * GetTransform());
  Renderer::CopyFrom(mColourBuffer, Colour);
}

void AbstractShape::Draw(CommandEncoder& command, const RenderState& renderState)
{
  auto pipeline = mDevice.CreateGraphicsPipeline(mPipeline, renderState);

  command.SetPipeline(PipelineBindPoint::Graphics, pipeline);
  command.SetBindGroup(PipelineBindPoint::Graphics, mPipelineLayout, mBindGroup);
  command.SetVertexBuffer(mVertexBuffer);
  command.Draw(mNumVertices);
}

Rectangle::Rectangle(Device& device, const glm::vec2& size)
    : AbstractShape(device,
                    SPIRV::Position_frag,
                    {{0.0f, 0.0f},
                     {size.x, 0.0f},
                     {0.0f, size.y},
                     {size.x, 0.0f},
                     {size.x, size.y},
                     {0.0f, size.y}})
{
}

IntRectangle::IntRectangle(Device& device, const glm::vec2& size)
    : AbstractShape(device,
                    SPIRV::IntPosition_frag,
                    {{0.0f, 0.0f},
                     {size.x, 0.0f},
                     {0.0f, size.y},
                     {size.x, 0.0f},
                     {size.x, size.y},
                     {0.0f, size.y}})
{
}

Mesh::Mesh(Device& device,
           VertexBuffer<glm::vec2>& vertexBuffer,
           IndexBuffer<std::uint32_t>& indexBuffer,
           Buffer<DrawIndexedIndirect>& parameters)
    : mDevice(device)
    , mMVPBuffer(device, MemoryUsage::CpuToGpu)
    , mColourBuffer(device, MemoryUsage::CpuToGpu)
    , mVertexBuffer(vertexBuffer)
    , mIndexBuffer(indexBuffer)
    , mParameters(parameters)
{
  SPIRV::Reflection reflectionVert(SPIRV::Position_vert);
  SPIRV::Reflection reflectionFrag(SPIRV::Position_frag);

  SPIRV::ShaderLayouts layout = {reflectionVert, reflectionFrag};

  mPipelineLayout = mDevice.CreatePipelineLayout(layout);
  auto bindGroupLayout = mDevice.CreateBindGroupLayout(layout);
  mBindGroup = mDevice.CreateBindGroup(bindGroupLayout, layout, {{mMVPBuffer}, {mColourBuffer}});

  auto vertexShader = device.CreateShaderModule(SPIRV::Position_vert);
  auto fragShader = device.CreateShaderModule(SPIRV::Position_frag);

  mPipeline = GraphicsPipelineDescriptor()
                  .Topology(PrimitiveTopology::LineList)
                  .Shader(vertexShader, ShaderStage::Vertex)
                  .Shader(fragShader, ShaderStage::Fragment)
                  .VertexAttribute(0, 0, Format::R32G32Sfloat, 0)
                  .VertexBinding(0, sizeof(glm::vec2))
                  .Layout(mPipelineLayout);
}

void Mesh::Initialize(const RenderState& renderState)
{
  mDevice.CreateGraphicsPipeline(mPipeline, renderState);
}

void Mesh::Update(const glm::mat4& projection, const glm::mat4& view)
{
  Transformable::Update();
  Renderer::CopyFrom(mMVPBuffer, projection * view * GetTransform());
  Renderer::CopyFrom(mColourBuffer, Colour);
}

void Mesh::Draw(CommandEncoder& command, const RenderState& renderState)
{
  auto pipeline = mDevice.CreateGraphicsPipeline(mPipeline, renderState);
  command.SetPipeline(PipelineBindPoint::Graphics, pipeline);
  command.SetVertexBuffer(mVertexBuffer);
  command.SetIndexBuffer(mIndexBuffer);
  command.SetBindGroup(PipelineBindPoint::Graphics, mPipelineLayout, mBindGroup);
  command.DrawIndexedIndirect(mParameters);
}

Ellipse::Ellipse(Device& device, const glm::vec2& radius)
    : mDevice(device)
    , mRadius(radius)
    , mMVPBuffer(device, MemoryUsage::CpuToGpu)
    , mColourBuffer(device, MemoryUsage::CpuToGpu)
    , mVertexBuffer(device, 6)
    , mSizeBuffer(device, MemoryUsage::CpuToGpu)
{
  VertexBuffer<glm::vec2> localVertices(device, 6, MemoryUsage::Cpu);
  std::vector<glm::vec2> vertices = {{-radius.x, -radius.y},
                                     {radius.x + 1.0f, -radius.y},
                                     {-radius.x, radius.y + 1.0f},
                                     {radius.x + 1.0f, -radius.y},
                                     {radius.x + 1.0f, radius.y + 1.0f},
                                     {-radius.x, radius.y + 1.0f}};
  Renderer::CopyFrom(localVertices, vertices);
  device.Execute([&](CommandEncoder& command) { mVertexBuffer.CopyFrom(command, localVertices); });

  SPIRV::Reflection reflectionVert(SPIRV::Ellipse_vert);
  SPIRV::Reflection reflectionFrag(SPIRV::Ellipse_frag);

  SPIRV::ShaderLayouts layout = {reflectionVert, reflectionFrag};

  auto bindGroupLayout = mDevice.CreateBindGroupLayout(layout);
  mPipelineLayout = mDevice.CreatePipelineLayout(layout);
  mBindGroup = mDevice.CreateBindGroup(
      bindGroupLayout, layout, {{mMVPBuffer}, {mSizeBuffer}, {mColourBuffer}});

  Handle::ShaderModule vertexShader = mDevice.CreateShaderModule(SPIRV::Ellipse_vert);
  Handle::ShaderModule fragShader = mDevice.CreateShaderModule(SPIRV::Ellipse_frag);

  mPipeline = GraphicsPipelineDescriptor()
                  .Shader(vertexShader, ShaderStage::Vertex)
                  .Shader(fragShader, ShaderStage::Fragment)
                  .VertexAttribute(0, 0, Format::R32G32Sfloat, 0)
                  .VertexBinding(0, sizeof(glm::vec2))
                  .Layout(mPipelineLayout);
}

Ellipse::~Ellipse() {}

void Ellipse::Initialize(const RenderState& renderState)
{
  mDevice.CreateGraphicsPipeline(mPipeline, renderState);
}

void Ellipse::Update(const glm::mat4& projection, const glm::mat4& view)
{
  Transformable::Update();
  Renderer::CopyFrom(mColourBuffer, Colour);
  Renderer::CopyFrom(mMVPBuffer, projection * view * GetTransform());

  Size size;
  glm::vec2 transformScale(glm::length(view[0]), glm::length(view[1]));
  size.radius = mRadius * Scale * transformScale;

  glm::mat4 rotation4 = glm::rotate(-glm::radians(Rotation), glm::vec3(0.0f, 0.0f, 1.0f));
  size.rotation[0] = rotation4[0];
  size.rotation[1] = rotation4[1];

  size.view.x = 1.0f / projection[0][0];
  size.view.y = 1.0f / projection[1][1];

  Renderer::CopyFrom(mSizeBuffer, size);
}

void Ellipse::Draw(CommandEncoder& command, const RenderState& renderState)
{
  auto pipeline = mDevice.CreateGraphicsPipeline(mPipeline, renderState);

  command.SetPipeline(PipelineBindPoint::Graphics, pipeline);
  command.SetBindGroup(PipelineBindPoint::Graphics, mPipelineLayout, mBindGroup);
  command.SetVertexBuffer(mVertexBuffer);
  command.Draw(6);
}

Clear::Clear(const glm::vec4& colour) : mColour(colour) {}

Clear::~Clear() {}

void Clear::Initialize(const RenderState& /*renderState*/) {}

void Clear::Update(const glm::mat4& /*projection*/, const glm::mat4& /*view*/) {}

void Clear::Draw(CommandEncoder& command, const RenderState& renderState)
{
  command.Clear({0, 0},
                {renderState.Width, renderState.Height},
                {mColour.r, mColour.g, mColour.b, mColour.a});
}

}  // namespace Renderer
}  // namespace Vortex
