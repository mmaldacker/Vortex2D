//
//  Shapes.cpp
//  Vortex2D
//

#include "Shapes.h"

#include <glm/gtx/transform.hpp>

#include <Vortex2D/Renderer/RenderTarget.h>

#include <limits>
#include <algorithm>

namespace Vortex2D { namespace Renderer {

vk::PrimitiveTopology GetTopology(Shape::Type type)
{
    switch (type)
    {
    case Shape::Type::POINTS:
        return vk::PrimitiveTopology::ePointList;
    case Shape::Type::TRIANGLES:
        return vk::PrimitiveTopology::eTriangleList;
    }
}

Shape::Shape(const Device& device, Type type, const Path& path)
    : mCount(path.size())
    , mProgram(device.GetDevice())
    , mVertexBuffer(device, path)
{
    vk::PipelineInputAssemblyStateCreateInfo inputAssembly;
    inputAssembly.setTopology(GetTopology(type));

    //VertexBuffer<float> vertexBuffer()

    vk::PipelineRasterizationStateCreateInfo rasterizationInfo;
    rasterizationInfo
            .setLineWidth(1.0f)
            .setCullMode(vk::CullModeFlagBits::eBack)
            .setFrontFace(vk::FrontFace::eClockwise);

    vk::PipelineMultisampleStateCreateInfo multisampleInfo;
    multisampleInfo
            .setRasterizationSamples(vk::SampleCountFlagBits::e1)
            .setMinSampleShading(1.0f);

    // TODO add blending

    vk::DynamicState dynamicStates[] = { vk::DynamicState::eViewport,
                                         vk::DynamicState::eScissor,
                                         vk::DynamicState::eBlendConstants };

    vk::PipelineDynamicStateCreateInfo dynamicInfo;
    dynamicInfo
            .setDynamicStateCount(3)
            .setPDynamicStates(dynamicStates);

    vk::UniquePipelineLayout pipelineLayout;
    // TODO fill in


    //vk::GraphicsPipelineCreateInfo graphicsPipelineInfo;
    //graphicsPipelineInfo.setStageCount(2)
    //        .set

}

void Shape::Render(const Device& device, RenderTarget & target)
{
    vk::CommandBufferBeginInfo beginInfo;
    beginInfo.setFlags(vk::CommandBufferUsageFlagBits::eSimultaneousUse);

    vk::CommandBuffer commandBuffer = device.CreateCommandBuffers(1).at(0);
    commandBuffer.begin(beginInfo);

    // TODO seems a bit redundant
    vk::ClearColorValue clearColour;
    clearColour.setFloat32({0.0f, 0.0f, 0.0f, 1.0f});

    vk::ClearValue clear(clearColour);

    vk::RenderPassBeginInfo renderPassInfo;
    renderPassInfo
            .setRenderPass(target.RenderPass)
            .setRenderArea({{0u,0u}, {target.Width, target.Height}})
            .setClearValueCount(1)
            .setPClearValues(&clear)
            .setFramebuffer(target.Framebuffer)
            .setRenderArea({{0u, 0u}, {target.Width, target.Height}});

    commandBuffer.beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);
    //commandBuffer.bindPipeline(mPipeline, )
    commandBuffer.draw(mCount, 1, 0, 0);
    commandBuffer.endRenderPass();
    commandBuffer.end();
}

Rectangle::Rectangle(const Device& device, const glm::vec2& size)
    : Shape(device, Type::TRIANGLES, {{0.0f, 0.0f}, {size.x, 0.0f}, {0.0f, size.y}, {size.x, 0.0f,}, {size.x, size.y}, {0.0f, size.y}})
{
}

Ellipse::Ellipse(const Device& device, const glm::vec2& radius)
{
}

void Ellipse::Render(const Device& device, RenderTarget & target)
{
    /*
    glm::vec2 transformScale(glm::length(transform[0]), glm::length(transform[1]));
    glm::vec2 radius = mRadius * (glm::vec2)Scale * transformScale;
    glm::mat4 rotation4 = glm::rotate(glm::radians((float)Rotation), glm::vec3(0.0f, 0.0f, 1.0f));
    glm::mat2 rotation(rotation4[0].xy, rotation4[1].xy);
    mProgram.Use()
            .Set("u_radius", radius)
            .Set("u_size", std::max(radius.x, radius.y))
            .Set("u_rotation", rotation);
    Shape::Render(target, transform);
    */
}

}}
