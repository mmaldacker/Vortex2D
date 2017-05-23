//
//  Shapes.cpp
//  Vortex2D
//

#include "Shapes.h"

#include <Vortex2D/Renderer/RenderTarget.h>

namespace Vortex2D { namespace Renderer {

Shape::Shape(const Device& device, const std::vector<glm::vec2>& vertices, const glm::vec4& colour)
    : mDevice(device.Handle())
    , mMVPBuffer(device,
                 vk::BufferUsageFlagBits::eUniformBuffer,
                 vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
                 sizeof(glm::mat4))
    , mColourBuffer(device,
                    vk::BufferUsageFlagBits::eUniformBuffer,
                    vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
                    sizeof(glm::vec4))
    , mVertexBuffer(device,
                    vk::BufferUsageFlagBits::eVertexBuffer,
                    vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
                    sizeof(glm::vec2) * vertices.size())
    , mNumVertices(vertices.size())
{
    mColourBuffer.CopyTo(colour);
    mVertexBuffer.CopyTo(vertices);

    static vk::DescriptorSetLayout descriptorLayout = DescriptorSetLayoutBuilder()
            .Binding(0, vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eVertex, 1)
            .Binding(1, vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eFragment, 1)
            .Create(device);

    mDescriptorSet = DescriptorSet(device.Handle(), descriptorLayout, device.DescriptorPool());

    DescriptorSetUpdater()
            .WriteDescriptorSet(mDescriptorSet)
            .WriteBuffers(0, 0, vk::DescriptorType::eUniformBuffer).Buffer(mMVPBuffer)
            .WriteBuffers(1, 0, vk::DescriptorType::eUniformBuffer).Buffer(mColourBuffer)
            .Update(device.Handle());

    // TODO should be static?
    mPipelineLayout = PipelineLayout()
            .DescriptorSetLayout(descriptorLayout)
            .Create(device.Handle());

    static vk::ShaderModule vertexShader = ShaderBuilder()
            .File("../Vortex2D/Position.vert.spv")
            .Create(device);

    static vk::ShaderModule fragShader = ShaderBuilder()
            .File("../Vortex2D/Position.frag.spv")
            .Create(device);

    mPipeline = GraphicsPipeline::Builder()
            .Shader(vertexShader, vk::ShaderStageFlagBits::eVertex)
            .Shader(fragShader, vk::ShaderStageFlagBits::eFragment)
            .VertexAttribute(0, 0, vk::Format::eR32G32Sfloat, 0)
            .VertexBinding(0, sizeof(glm::vec2))
            .Layout(mPipelineLayout);
}

void Shape::Initialize(const RenderState& renderState)
{
    mPipeline.Create(mDevice, renderState);
}

void Shape::Update(const glm::mat4& model, const glm::mat4& view)
{
    mMVPBuffer.CopyTo(model * view * GetTransform());
}

void Shape::Draw(vk::CommandBuffer commandBuffer, const RenderState& renderState)
{
    mPipeline.Bind(commandBuffer, renderState);
    commandBuffer.bindVertexBuffers(0, {mVertexBuffer}, {0ul});
    commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, mPipelineLayout, 0, {mDescriptorSet}, {});
    commandBuffer.draw(mNumVertices, 1, 0, 0);
}

Rectangle::Rectangle(const Device& device, const glm::vec2& size, const glm::vec4& colour)
    : Shape(device, {{0.0f, 0.0f},
                     {size.x, 0.0f},
                     {0.0f, size.y},
                     {size.x, 0.0f,},
                     {size.x, size.y},
                     {0.0f, size.y}}, colour)
{
}


/*
glm::vec2 transformScale(glm::length(transform[0]), glm::length(transform[1]));
glm::vec2 radius = mRadius * (glm::vec2)Scale * transformScale;
-
glm::mat4 rotation4 = glm::rotate(glm::radians((float)Rotation), glm::vec3(0.0f, 0.0f, 1.0f));
glm::mat2 rotation(rotation4[0].xy, rotation4[1].xy);
mProgram.Use()
        .Set("u_radius", radius)
        .Set("u_size", std::max(radius.x, radius.y))
        .Set("u_rotation", rotation);
Shape::Render(target, transform);
*/

Clear::Clear(uint32_t width, uint32_t height, const glm::vec4& colour)
    : mWidth(width)
    , mHeight(height)
    , mColour(colour)
{

}

void Clear::Draw(vk::CommandBuffer commandBuffer)
{
    auto clearValue = vk::ClearValue()
            .setColor(std::array<float, 4>{{mColour.r, mColour.g, mColour.b, mColour.a}});

    auto clearAttachement = vk::ClearAttachment()
            .setAspectMask(vk::ImageAspectFlagBits::eColor)
            .setClearValue(clearValue);

    auto clearRect = vk::ClearRect()
            .setRect({{0, 0}, {mWidth, mHeight}})
            .setLayerCount(1);

    commandBuffer.clearAttachments({clearAttachement}, {clearRect});
}

}}
