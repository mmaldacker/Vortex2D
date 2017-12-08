//
//  Sprite.cpp
//  Vortex2D
//

#include "Sprite.h"

#include <Vortex2D/Renderer/RenderTarget.h>
#include <Vortex2D/SPIRV/Reflection.h>
#include <Vortex2D/Renderer/CommandBuffer.h>

namespace Vortex2D { namespace Renderer {

AbstractSprite::AbstractSprite(const Device& device, const std::string& fragShaderName, Texture& texture)
    : mDevice(device)
    , mMVPBuffer(device)
    , mVertexBuffer(device, 6)
{
    VertexBuffer<Vertex> localBuffer(device, 6, VMA_MEMORY_USAGE_CPU_ONLY);
    std::vector<Vertex> vertices = {
        {{0.0f, 0.0f}, {0.0f, 0.0f}},
        {{1.0f, 0.0f}, {texture.GetWidth(), 0.0f}},
        {{0.0f, 1.0f}, {0.0f, texture.GetHeight()}},
        {{1.0f, 0.0f}, {texture.GetWidth(), 0.0f}},
        {{1.0f, 1.0f}, {texture.GetWidth(), texture.GetHeight()}},
        {{0.0f, 1.0f}, {0.0f, texture.GetHeight()}}
    };

    Renderer::CopyFrom(localBuffer, vertices);
    ExecuteCommand(device, [&](vk::CommandBuffer commandBuffer)
    {
        mVertexBuffer.CopyFrom(commandBuffer, localBuffer);
    });

    SPIRV::Reflection reflectionVert(device.GetShaderSPIRV("../Vortex2D/TexturePosition.vert.spv"));
    SPIRV::Reflection reflectionFrag(device.GetShaderSPIRV(fragShaderName));

    PipelineLayout layout = {{reflectionVert, reflectionFrag}};
    mDescriptorSet = device.GetLayoutManager().MakeDescriptorSet(layout);

    // TODO add as parameter
    mSampler = SamplerBuilder().Filter(vk::Filter::eLinear).Create(device.Handle());

    Bind(device, *mDescriptorSet.descriptorSet, layout, {{mMVPBuffer, 0}, {*mSampler, texture, 1}});

    vk::ShaderModule vertexShader = device.GetShaderModule("../Vortex2D/TexturePosition.vert.spv");
    vk::ShaderModule fragShader = device.GetShaderModule(fragShaderName);

    mPipeline = GraphicsPipeline::Builder()
            .Shader(vertexShader, vk::ShaderStageFlagBits::eVertex)
            .Shader(fragShader, vk::ShaderStageFlagBits::eFragment)
            .VertexAttribute(0, 0, vk::Format::eR32G32Sfloat, offsetof(Vertex, pos))
            .VertexAttribute(1, 0, vk::Format::eR32G32Sfloat, offsetof(Vertex, uv))
            .VertexBinding(0, sizeof(Vertex))
            .Layout(mDescriptorSet.pipelineLayout);
}

void AbstractSprite::Update(const glm::mat4& projection, const glm::mat4& view)
{
    Renderer::CopyFrom(mMVPBuffer, projection * view * GetTransform());
}

void AbstractSprite::Initialize(const RenderState& renderState)
{
    mPipeline.Create(mDevice.Handle(), renderState);
}

void AbstractSprite::Draw(vk::CommandBuffer commandBuffer, const RenderState& renderState)
{
    mPipeline.Bind(commandBuffer, renderState);
    commandBuffer.bindVertexBuffers(0, {mVertexBuffer.Handle()}, {0ul});
    commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics,
                                     mDescriptorSet.pipelineLayout, 0, {*mDescriptorSet.descriptorSet}, {});
    commandBuffer.draw(6, 1, 0, 0);
}

Sprite::Sprite(const Device& device, Texture& texture)
    : AbstractSprite(device, "../Vortex2D/TexturePosition.frag.spv", texture)
{

}

}}
