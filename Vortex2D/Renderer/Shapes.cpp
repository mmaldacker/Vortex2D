//
//  Shapes.cpp
//  Vortex2D
//

#include "Shapes.h"

#include <glm/gtx/transform.hpp>

#include <Vortex2D/Renderer/RenderTarget.h>
#include <Vortex2D/SPIRV/Reflection.h>

namespace Vortex2D { namespace Renderer {

AbstractShape::AbstractShape(const Device& device,
                             const std::string& fragName,
                             const std::vector<glm::vec2>& vertices,
                             const glm::vec4& colour)
    : mDevice(device.Handle())
    , mMVPBuffer(device, vk::BufferUsageFlagBits::eUniformBuffer, true, sizeof(glm::mat4))
    , mColourBuffer(device, vk::BufferUsageFlagBits::eUniformBuffer, true, sizeof(glm::vec4))
    , mVertexBuffer(device, vk::BufferUsageFlagBits::eVertexBuffer, true, sizeof(glm::vec2) * vertices.size())
    , mNumVertices(vertices.size())
{
    mColourBuffer.CopyFrom(colour);
    mVertexBuffer.CopyFrom(vertices);

    SPIRV::Reflection reflectionVert(device.GetShaderSPIRV("../Vortex2D/Position.vert.spv"));
    SPIRV::Reflection reflectionFrag(device.GetShaderSPIRV(fragName));

    DescriptorSetLayoutBuilder descriptorSetLayoutBuilder;
    reflectionVert.AddBindings(descriptorSetLayoutBuilder);
    reflectionFrag.AddBindings(descriptorSetLayoutBuilder);

    vk::DescriptorSetLayout descriptorLayout = descriptorSetLayoutBuilder.Create(device);
    mDescriptorSet = MakeDescriptorSet(device, descriptorLayout);

    DescriptorSetUpdater(*mDescriptorSet)
            .Bind(reflectionVert.GetDescriptorTypesMap(), {{mMVPBuffer, 0}})
            .Bind(reflectionFrag.GetDescriptorTypesMap(), {{mColourBuffer, 1}})
            .Update(device.Handle());

    mPipelineLayout = PipelineLayoutBuilder()
            .DescriptorSetLayout(descriptorLayout)
            .Create(device.Handle());

    vk::ShaderModule vertexShader = device.GetShaderModule("../Vortex2D/Position.vert.spv");
    vk::ShaderModule fragShader = device.GetShaderModule(fragName);

    mPipeline = GraphicsPipeline::Builder()
            .Shader(vertexShader, vk::ShaderStageFlagBits::eVertex)
            .Shader(fragShader, vk::ShaderStageFlagBits::eFragment)
            .VertexAttribute(0, 0, vk::Format::eR32G32Sfloat, 0)
            .VertexBinding(0, sizeof(glm::vec2))
            .Layout(*mPipelineLayout);
}

void AbstractShape::Initialize(const RenderState& renderState)
{
    mPipeline.Create(mDevice, renderState);
}

void AbstractShape::Update(const glm::mat4& projection, const glm::mat4& view)
{
    mMVPBuffer.CopyFrom(projection * view * GetTransform());
}

void AbstractShape::Draw(vk::CommandBuffer commandBuffer, const RenderState& renderState)
{
    mPipeline.Bind(commandBuffer, renderState);
    commandBuffer.bindVertexBuffers(0, {mVertexBuffer}, {0ul});
    commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, *mPipelineLayout, 0, {*mDescriptorSet}, {});
    commandBuffer.draw(mNumVertices, 1, 0, 0);
}

Rectangle::Rectangle(const Device& device, const glm::vec2& size, const glm::vec4& colour)
    : AbstractShape(device, "../Vortex2D/Position.frag.spv",
                    {{0.0f, 0.0f},
                     {size.x, 0.0f},
                     {0.0f, size.y},
                     {size.x, 0.0f,},
                     {size.x, size.y},
                     {0.0f, size.y}},
                    colour)
{
}

IntRectangle::IntRectangle(const Device& device, const glm::vec2& size, const glm::ivec4& colour)
    : AbstractShape(device, "../Vortex2D/IntPosition.frag.spv",
                    {{0.0f, 0.0f},
                     {size.x, 0.0f},
                     {0.0f, size.y},
                     {size.x, 0.0f,},
                     {size.x, size.y},
                     {0.0f, size.y}},
                    colour)
{
    mColourBuffer.CopyFrom(colour);
}


Ellipse::Ellipse(const Device& device, const glm::vec2& radius, const glm::vec4& colour)
    : mDevice(device.Handle())
    , mRadius(radius)
    , mMVPBuffer(device, vk::BufferUsageFlagBits::eUniformBuffer, true, sizeof(glm::mat4))
    , mColourBuffer(device, vk::BufferUsageFlagBits::eUniformBuffer, true, sizeof(glm::vec4))
    , mVertexBuffer(device, vk::BufferUsageFlagBits::eVertexBuffer, true, 6*sizeof(glm::vec2))
    , mSizeBuffer(device, vk::BufferUsageFlagBits::eUniformBuffer, true, sizeof(Size))
{
    mColourBuffer.CopyFrom(colour);
    mVertexBuffer.CopyFrom(std::vector<glm::vec2>{{-radius.x, -radius.y},
                            {radius.x + 1.0f, -radius.y},
                            {-radius.x, radius.y + 1.0f},
                            {radius.x + 1.0f, -radius.y},
                            {radius.x + 1.0f, radius.y + 1.0f},
                            {-radius.x, radius.y + 1.0f}});

    static vk::DescriptorSetLayout descriptorLayout = DescriptorSetLayoutBuilder()
            .Binding(0, vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eVertex, 1)
            .Binding(1, vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eFragment, 1)
            .Binding(2, vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eFragment, 1)
            .Create(device);

    mDescriptorSet = MakeDescriptorSet(device, descriptorLayout);

    DescriptorSetUpdater(*mDescriptorSet)
            .WriteBuffers(0, 0, vk::DescriptorType::eUniformBuffer).Buffer(mMVPBuffer)
            .WriteBuffers(1, 0, vk::DescriptorType::eUniformBuffer).Buffer(mSizeBuffer)
            .WriteBuffers(2, 0, vk::DescriptorType::eUniformBuffer).Buffer(mColourBuffer)
            .Update(device.Handle());

    mPipelineLayout = PipelineLayoutBuilder()
            .DescriptorSetLayout(descriptorLayout)
            .Create(device.Handle());

    vk::ShaderModule vertexShader = device.GetShaderModule("../Vortex2D/Ellipse.vert.spv");
    vk::ShaderModule fragShader = device.GetShaderModule("../Vortex2D/Ellipse.frag.spv");

    mPipeline = GraphicsPipeline::Builder()
            .Shader(vertexShader, vk::ShaderStageFlagBits::eVertex)
            .Shader(fragShader, vk::ShaderStageFlagBits::eFragment)
            .VertexAttribute(0, 0, vk::Format::eR32G32Sfloat, 0)
            .VertexBinding(0, sizeof(glm::vec2))
            .Layout(*mPipelineLayout);
}

void Ellipse::Initialize(const RenderState& renderState)
{
    mPipeline.Create(mDevice, renderState);
}

void Ellipse::Update(const glm::mat4& projection, const glm::mat4& view)
{
    mMVPBuffer.CopyFrom(projection * view * GetTransform());

    Size size;
    glm::vec2 transformScale(glm::length(view[0]), glm::length(view[1]));
    size.radius = mRadius * (glm::vec2)Scale * transformScale;

    glm::mat4 rotation4 = glm::rotate(-glm::radians((float)Rotation), glm::vec3(0.0f, 0.0f, 1.0f));
    size.rotation[0] = rotation4[0];
    size.rotation[1] = rotation4[1];

    size.view.x = 1.0f / projection[0][0];
    size.view.y = 1.0f / projection[1][1];

    mSizeBuffer.CopyFrom(size);
}

void Ellipse::Draw(vk::CommandBuffer commandBuffer, const RenderState& renderState)
{
    mPipeline.Bind(commandBuffer, renderState);
    commandBuffer.bindVertexBuffers(0, {mVertexBuffer}, {0ul});
    commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, *mPipelineLayout, 0, {*mDescriptorSet}, {});
    commandBuffer.draw(6, 1, 0, 0);
}

Clear::Clear(uint32_t width, uint32_t height, const glm::vec4& colour)
    : mWidth(width)
    , mHeight(height)
    , mColour(colour)
{
}

void Clear::Initialize(const RenderState& renderState)
{

}

void Clear::Update(const glm::mat4& projection, const glm::mat4& view)
{

}

void Clear::Draw(vk::CommandBuffer commandBuffer, const RenderState& renderState)
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
