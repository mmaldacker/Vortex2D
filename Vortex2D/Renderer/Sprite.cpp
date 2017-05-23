//
//  Sprite.cpp
//  Vortex2D
//

#include "Sprite.h"

#include <Vortex2D/Renderer/RenderTarget.h>

namespace Vortex2D { namespace Renderer {

Sprite::Sprite(const Device& device, const Texture& texture)
    : mDevice(device.Handle())
    , mMVPBuffer(device,
                 vk::BufferUsageFlagBits::eUniformBuffer,
                 vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
                 sizeof(glm::mat4))
    , mVertexBuffer(device,
                    vk::BufferUsageFlagBits::eVertexBuffer,
                    vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
                    sizeof(glm::vec2) * 6)
{
    Vertex vertices[] = {
        {{0.0f, 0.0f}, {0.0f, 0.0f}},
        {{1.0f, 0.0f}, {texture.GetWidth(), 0.0f}},
        {{0.0f, 1.0f}, {0.0f, texture.GetHeight()}},
        {{1.0f, 0.0f}, {texture.GetWidth(), 0.0f}},
        {{1.0f, 1.0f}, {texture.GetWidth(), texture.GetHeight()}},
        {{0.0f, 1.0f}, {0.0f, texture.GetHeight()}}
    };

    mVertexBuffer.CopyTo(vertices);

    static vk::DescriptorSetLayout descriptorLayout = DescriptorSetLayoutBuilder()
            .Binding(0, vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eVertex, 1)
            .Binding(1, vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eFragment, 1)
            .Create(device);

    mDescriptorSet = DescriptorSet(device.Handle(), descriptorLayout, device.DescriptorPool());

    // TODO add as parameter
    mSampler = SamplerBuilder().Create(device.Handle());

    DescriptorSetUpdater()
            .WriteDescriptorSet(mDescriptorSet)
            .WriteBuffers(0, 0, vk::DescriptorType::eUniformBuffer)
            .Buffer(mMVPBuffer)
            .WriteImages(1, 0, vk::DescriptorType::eCombinedImageSampler)
            .Image(*mSampler, texture, vk::ImageLayout::eShaderReadOnlyOptimal)
            .Update(device.Handle());

    // TODO should be static?
    mPipelineLayout = PipelineLayout()
            .DescriptorSetLayout(descriptorLayout)
            .Create(device.Handle());

    static vk::ShaderModule vertexShader = ShaderBuilder()
            .File("../TexturePosition.vert.spv")
            .Create(device);

    static vk::ShaderModule fragShader = ShaderBuilder()
            .File("../TexturePosition.frag.spv")
            .Create(device);

    mPipeline = GraphicsPipeline::Builder()
            .Shader(vertexShader, vk::ShaderStageFlagBits::eVertex)
            .Shader(fragShader, vk::ShaderStageFlagBits::eFragment)
            .VertexAttribute(0, 0, vk::Format::eR32G32Sfloat, offsetof(Vertex, pos))
            .VertexAttribute(1, 0, vk::Format::eR32G32Sfloat, offsetof(Vertex, uv))
            .VertexBinding(0, sizeof(Vertex))
            .Layout(mPipelineLayout);

}

void Sprite::Update(const glm::mat4& model, const glm::mat4& view)
{
    mMVPBuffer.CopyTo(model * view * GetTransform());
}

void Sprite::Initialize(const RenderState& renderState)
{
    mPipeline.Create(mDevice, renderState);
}

void Sprite::Draw(vk::CommandBuffer commandBuffer, const RenderState& renderState)
{
    mPipeline.Bind(commandBuffer, renderState);
    commandBuffer.bindVertexBuffers(0, {mVertexBuffer}, {0ul});
    commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, mPipelineLayout, 0, {mDescriptorSet}, {});
    commandBuffer.draw(6, 1, 0, 0);
}

}}
