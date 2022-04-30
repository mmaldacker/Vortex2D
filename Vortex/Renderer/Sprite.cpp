//
//  Sprite.cpp
//  Vortex
//

#include "Sprite.h"

#include <Vortex/Renderer/CommandBuffer.h>
#include <Vortex/Renderer/RenderTarget.h>
#include <Vortex/SPIRV/Reflection.h>

#include "vortex_generated_spirv.h"

namespace Vortex
{
namespace Renderer
{
AbstractSprite::AbstractSprite(Device& device, const SpirvBinary& fragShaderName, Texture& texture)
    : mDevice(device)
    , mMVPBuffer(device, MemoryUsage::CpuToGpu)
    , mVertexBuffer(device, 6)
    , mColourBuffer(device, MemoryUsage::CpuToGpu)
    , mSampler(device, Sampler::AddressMode::Repeat, Sampler::Filter::Linear)
{
  VertexBuffer<Vertex> localBuffer(device, 6, MemoryUsage::Cpu);
  std::vector<Vertex> vertices = {{{0.0f, 0.0f}, {0.0f, 0.0f}},
                                  {{1.0f, 0.0f}, {texture.GetWidth(), 0.0f}},
                                  {{0.0f, 1.0f}, {0.0f, texture.GetHeight()}},
                                  {{1.0f, 0.0f}, {texture.GetWidth(), 0.0f}},
                                  {{1.0f, 1.0f}, {texture.GetWidth(), texture.GetHeight()}},
                                  {{0.0f, 1.0f}, {0.0f, texture.GetHeight()}}};

  Renderer::CopyFrom(localBuffer, vertices);
  device.Execute([&](CommandEncoder& command) { mVertexBuffer.CopyFrom(command, localBuffer); });

  SPIRV::Reflection reflectionVert(SPIRV::TexturePosition_vert);
  SPIRV::Reflection reflectionFrag(fragShaderName);

  SPIRV::ShaderLayouts layout = {reflectionVert, reflectionFrag};

  mPipelineLayout = mDevice.CreatePipelineLayout(layout);
  auto bindGroupLayout = mDevice.CreateBindGroupLayout(layout);
  mBindGroup = mDevice.CreateBindGroup(
      bindGroupLayout, layout, {{mMVPBuffer, 0}, {mSampler, texture, 1}, {mColourBuffer, 2}});

  Handle::ShaderModule vertexShader = mDevice.CreateShaderModule(SPIRV::TexturePosition_vert);
  Handle::ShaderModule fragShader = mDevice.CreateShaderModule(fragShaderName);

  mPipeline = GraphicsPipelineDescriptor()
                  .Shader(vertexShader, ShaderStage::Vertex)
                  .Shader(fragShader, ShaderStage::Fragment)
                  .VertexAttribute(0, 0, Format::R32G32Sfloat, offsetof(Vertex, pos))
                  .VertexAttribute(1, 0, Format::R32G32Sfloat, offsetof(Vertex, uv))
                  .VertexBinding(0, sizeof(Vertex))
                  .Layout(mPipelineLayout);
}

AbstractSprite::AbstractSprite(AbstractSprite&& other)
    : mDevice(other.mDevice)
    , mMVPBuffer(std::move(other.mMVPBuffer))
    , mVertexBuffer(std::move(other.mVertexBuffer))
    , mColourBuffer(std::move(other.mColourBuffer))
    , mSampler(std::move(other.mSampler))
    , mPipelineLayout(std::move(other.mPipelineLayout))
    , mBindGroup(std::move(other.mBindGroup))
    , mPipeline(std::move(other.mPipeline))
{
}

AbstractSprite::~AbstractSprite() {}

void AbstractSprite::Update(const glm::mat4& projection, const glm::mat4& view)
{
  Transformable::Update();
  Renderer::CopyFrom(mColourBuffer, Colour);
  Renderer::CopyFrom(mMVPBuffer, projection * view * GetTransform());
}

void AbstractSprite::Initialize(const RenderState& renderState)
{
  mDevice.CreateGraphicsPipeline(mPipeline, renderState);
}

void AbstractSprite::Draw(CommandEncoder& command, const RenderState& renderState)
{
  auto pipeline = mDevice.CreateGraphicsPipeline(mPipeline, renderState);

  command.SetPipeline(PipelineBindPoint::Graphics, pipeline);
  command.SetBindGroup(PipelineBindPoint::Graphics, mPipelineLayout, mBindGroup);
  command.SetVertexBuffer(mVertexBuffer);
  command.Draw(6);
}

Sprite::Sprite(Device& device, Texture& texture)
    : AbstractSprite(device, SPIRV::TexturePosition_frag, texture)
{
}

}  // namespace Renderer
}  // namespace Vortex
