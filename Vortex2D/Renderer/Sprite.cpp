//
//  Sprite.cpp
//  Vortex2D
//

#include "Sprite.h"

#include <Vortex2D/Renderer/RenderTarget.h>
#include <Vortex2D/SPIRV/Reflection.h>

namespace Vortex2D { namespace Renderer {

AbstractSprite::AbstractSprite(const Device& device, const std::string& fragShaderName, const Texture& texture)
    : mDevice(device.Handle())
    , mMVPBuffer(device, vk::BufferUsageFlagBits::eUniformBuffer, true, sizeof(glm::mat4))
    , mVertexBuffer(device, vk::BufferUsageFlagBits::eVertexBuffer, true, sizeof(Vertex) * 6)
{
    Vertex vertices[] = {
        {{0.0f, 0.0f}, {0.0f, 0.0f}},
        {{1.0f, 0.0f}, {texture.GetWidth(), 0.0f}},
        {{0.0f, 1.0f}, {0.0f, texture.GetHeight()}},
        {{1.0f, 0.0f}, {texture.GetWidth(), 0.0f}},
        {{1.0f, 1.0f}, {texture.GetWidth(), texture.GetHeight()}},
        {{0.0f, 1.0f}, {0.0f, texture.GetHeight()}}
    };

    mVertexBuffer.CopyFrom(vertices);

    static vk::DescriptorSetLayout descriptorLayout = DescriptorSetLayoutBuilder()
            .Binding(0, vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eVertex, 1)
            .Binding(1, vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eFragment, 1)
            .Create(device);

    mDescriptorSet = MakeDescriptorSet(device, descriptorLayout);

    // TODO add as parameter
    mSampler = SamplerBuilder().Filter(vk::Filter::eLinear).Create(device.Handle());

    DescriptorSetUpdater(*mDescriptorSet)
            .WriteBuffers(0, 0, vk::DescriptorType::eUniformBuffer).Buffer(mMVPBuffer)
            .WriteImages(1, 0, vk::DescriptorType::eCombinedImageSampler).Image(*mSampler, texture, vk::ImageLayout::eShaderReadOnlyOptimal)
            .Update(device.Handle());

    auto pipelineLayoutBuilder = PipelineLayoutBuilder()
            .DescriptorSetLayout(descriptorLayout);

    SPIRV::Reflection reflection(device.GetShaderSPIRV(fragShaderName));
    unsigned pushConstantSize = reflection.GetPushConstantsSize();
    if (pushConstantSize > 0)
    {
        pipelineLayoutBuilder.PushConstantRange({vk::ShaderStageFlagBits::eFragment, 0, pushConstantSize});
    }

    mPipelineLayout = pipelineLayoutBuilder.Create(device.Handle());

    vk::ShaderModule vertexShader = device.GetShaderModule("../Vortex2D/TexturePosition.vert.spv");
    vk::ShaderModule fragShader = device.GetShaderModule(fragShaderName);

    mPipeline = GraphicsPipeline::Builder()
            .Shader(vertexShader, vk::ShaderStageFlagBits::eVertex)
            .Shader(fragShader, vk::ShaderStageFlagBits::eFragment)
            .VertexAttribute(0, 0, vk::Format::eR32G32Sfloat, offsetof(Vertex, pos))
            .VertexAttribute(1, 0, vk::Format::eR32G32Sfloat, offsetof(Vertex, uv))
            .VertexBinding(0, sizeof(Vertex))
            .Layout(*mPipelineLayout);

}

void AbstractSprite::Update(const glm::mat4& projection, const glm::mat4& view)
{
    mMVPBuffer.CopyFrom(projection * view * GetTransform());
}

void AbstractSprite::Initialize(const RenderState& renderState)
{
    mPipeline.Create(mDevice, renderState);
}

void AbstractSprite::Draw(vk::CommandBuffer commandBuffer, const RenderState& renderState)
{
    mPipeline.Bind(commandBuffer, renderState);
    commandBuffer.bindVertexBuffers(0, {mVertexBuffer}, {0ul});
    commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, *mPipelineLayout, 0, {*mDescriptorSet}, {});
    commandBuffer.draw(6, 1, 0, 0);
}

Sprite::Sprite(const Device& device, const Texture& texture)
    : AbstractSprite(device, "../Vortex2D/TexturePosition.frag.spv", texture)
{

}

}}
