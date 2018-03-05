//
//  Shapes.cpp
//  Vortex2D
//

#include "Shapes.h"

#include <glm/gtx/transform.hpp>

#include <Vortex2D/Renderer/RenderTarget.h>
#include <Vortex2D/Renderer/CommandBuffer.h>
#include <Vortex2D/SPIRV/Reflection.h>

#include "vortex2d_generated_spirv.h"

namespace Vortex2D { namespace Renderer {

AbstractShape::AbstractShape(const Device& device,
                             const SpirvBinary& fragName,
                             const std::vector<glm::vec2>& vertices,
                             const glm::vec4& colour)
    : mDevice(device)
    , mMVPBuffer(device, VMA_MEMORY_USAGE_CPU_TO_GPU)
    , mColourBuffer(device, VMA_MEMORY_USAGE_CPU_TO_GPU)
    , mVertexBuffer(device, vertices.size())
    , mNumVertices(vertices.size())
{
    Renderer::CopyFrom(mColourBuffer, colour);

    VertexBuffer<glm::vec2> localVertices(device, vertices.size(), VMA_MEMORY_USAGE_CPU_ONLY);
    Renderer::CopyFrom(localVertices, vertices);
    ExecuteCommand(device, [&](vk::CommandBuffer commandBuffer)
    {
        mVertexBuffer.CopyFrom(commandBuffer, localVertices);
    });

    SPIRV::Reflection reflectionVert(SPIRV::Position_vert);
    SPIRV::Reflection reflectionFrag(fragName);

    PipelineLayout layout = {{reflectionVert, reflectionFrag}};
    mDescriptorSet = device.GetLayoutManager().MakeDescriptorSet(layout);
    Bind(device, *mDescriptorSet.descriptorSet, layout, {{mMVPBuffer, 0}, {mColourBuffer, 1}});

    vk::ShaderModule vertexShader = device.GetShaderModule(SPIRV::Position_vert);
    vk::ShaderModule fragShader = device.GetShaderModule(fragName);

    mPipeline = GraphicsPipeline::Builder()
            .Shader(vertexShader, vk::ShaderStageFlagBits::eVertex)
            .Shader(fragShader, vk::ShaderStageFlagBits::eFragment)
            .VertexAttribute(0, 0, vk::Format::eR32G32Sfloat, 0)
            .VertexBinding(0, sizeof(glm::vec2))
            .Layout(mDescriptorSet.pipelineLayout);
}

void AbstractShape::Initialize(const RenderState& renderState)
{
    mPipeline.Create(mDevice.Handle(), renderState);
}

void AbstractShape::Update(const glm::mat4& projection, const glm::mat4& view)
{
    Renderer::CopyFrom(mMVPBuffer, projection * view * GetTransform());
    // TODO no way to update colour
}

void AbstractShape::Draw(vk::CommandBuffer commandBuffer, const RenderState& renderState)
{
    mPipeline.Bind(commandBuffer, renderState);
    commandBuffer.bindVertexBuffers(0, {mVertexBuffer.Handle()}, {0ul});
    commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics,
                                     mDescriptorSet.pipelineLayout, 0, {*mDescriptorSet.descriptorSet}, {});
    commandBuffer.draw(mNumVertices, 1, 0, 0);
}

Rectangle::Rectangle(const Device& device, const glm::vec2& size, const glm::vec4& colour)
    : AbstractShape(device, SPIRV::Position_frag,
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
    : AbstractShape(device, SPIRV::IntPosition_frag,
                    {{0.0f, 0.0f},
                     {size.x, 0.0f},
                     {0.0f, size.y},
                     {size.x, 0.0f,},
                     {size.x, size.y},
                     {0.0f, size.y}},
                    colour)
{
    UniformBuffer<glm::ivec4> localColour(device, VMA_MEMORY_USAGE_CPU_ONLY);
    CopyFrom(localColour, colour);
    ExecuteCommand(device, [&](vk::CommandBuffer commandBuffer)
    {
        mColourBuffer.CopyFrom(commandBuffer, localColour);
    });
}

void IntRectangle::Update(const glm::mat4& projection, const glm::mat4& view)
{
    Renderer::CopyFrom(mMVPBuffer, projection * view * GetTransform());
}

Ellipse::Ellipse(const Device& device, const glm::vec2& radius, const glm::vec4& colour)
    : mDevice(device)
    , mRadius(radius)
    , mMVPBuffer(device, VMA_MEMORY_USAGE_CPU_TO_GPU)
    , mColourBuffer(device, VMA_MEMORY_USAGE_CPU_TO_GPU)
    , mVertexBuffer(device, 6)
    , mSizeBuffer(device, VMA_MEMORY_USAGE_CPU_TO_GPU)
{
    Renderer::CopyFrom(mColourBuffer, colour);

    VertexBuffer<glm::vec2> localVertices(device, 6, VMA_MEMORY_USAGE_CPU_ONLY);
    std::vector<glm::vec2> vertices = {{-radius.x, -radius.y},
                                {radius.x + 1.0f, -radius.y},
                                {-radius.x, radius.y + 1.0f},
                                {radius.x + 1.0f, -radius.y},
                                {radius.x + 1.0f, radius.y + 1.0f},
                                {-radius.x, radius.y + 1.0f}};
    Renderer::CopyFrom(localVertices, vertices);
    ExecuteCommand(device, [&](vk::CommandBuffer commandBuffer)
    {
        mVertexBuffer.CopyFrom(commandBuffer, localVertices);
    });

    SPIRV::Reflection reflectionVert(SPIRV::Ellipse_vert);
    SPIRV::Reflection reflectionFrag(SPIRV::Ellipse_frag);

    Renderer::PipelineLayout layout = {{reflectionVert, reflectionFrag}};
    mDescriptorSet = device.GetLayoutManager().MakeDescriptorSet(layout);
    Bind(device, *mDescriptorSet.descriptorSet, layout, {{mMVPBuffer}, {mSizeBuffer}, {mColourBuffer}});

    vk::ShaderModule vertexShader = device.GetShaderModule(SPIRV::Ellipse_vert);
    vk::ShaderModule fragShader = device.GetShaderModule(SPIRV::Ellipse_frag);

    mPipeline = GraphicsPipeline::Builder()
            .Shader(vertexShader, vk::ShaderStageFlagBits::eVertex)
            .Shader(fragShader, vk::ShaderStageFlagBits::eFragment)
            .VertexAttribute(0, 0, vk::Format::eR32G32Sfloat, 0)
            .VertexBinding(0, sizeof(glm::vec2))
            .Layout(mDescriptorSet.pipelineLayout);
}

void Ellipse::Initialize(const RenderState& renderState)
{
    mPipeline.Create(mDevice.Handle(), renderState);
}

void Ellipse::Update(const glm::mat4& projection, const glm::mat4& view)
{
    Renderer::CopyFrom(mMVPBuffer, projection * view * GetTransform());

    Size size;
    glm::vec2 transformScale(glm::length(view[0]), glm::length(view[1]));
    size.radius = mRadius * (glm::vec2)Scale * transformScale;

    glm::mat4 rotation4 = glm::rotate(-glm::radians((float)Rotation), glm::vec3(0.0f, 0.0f, 1.0f));
    size.rotation[0] = rotation4[0];
    size.rotation[1] = rotation4[1];

    size.view.x = 1.0f / projection[0][0];
    size.view.y = 1.0f / projection[1][1];

    Renderer::CopyFrom(mSizeBuffer, size);

    // TODO no way to update colour
}

void Ellipse::Draw(vk::CommandBuffer commandBuffer, const RenderState& renderState)
{
    mPipeline.Bind(commandBuffer, renderState);
    commandBuffer.bindVertexBuffers(0, {mVertexBuffer.Handle()}, {0ul});
    commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics,
                                     mDescriptorSet.pipelineLayout, 0, {*mDescriptorSet.descriptorSet}, {});
    commandBuffer.draw(6, 1, 0, 0);
}

Clear::Clear(const glm::vec4& colour)
    : mColour(colour)
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
            .setRect({{0, 0}, {renderState.Width, renderState.Height}})
            .setLayerCount(1);

    commandBuffer.clearAttachments({clearAttachement}, {clearRect});
}

}}
