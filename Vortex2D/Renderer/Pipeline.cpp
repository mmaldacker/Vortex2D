//
//  Pipeline.cpp
//  Vortex2D
//

#include "Pipeline.h"

#include <algorithm>

namespace Vortex2D { namespace Renderer {

PipelineLayoutBuilder& PipelineLayoutBuilder::DescriptorSetLayout(vk::DescriptorSetLayout layout)
{
    mLayouts.push_back(layout);
    return *this;
}

PipelineLayoutBuilder& PipelineLayoutBuilder::PushConstantRange(vk::PushConstantRange range)
{
    mPushConstantRanges.push_back(range);
    return *this;
}

vk::UniquePipelineLayout PipelineLayoutBuilder::Create(vk::Device device)
{
    auto pipelineLayoutInfo = vk::PipelineLayoutCreateInfo()
            .setSetLayoutCount((uint32_t)mLayouts.size())
            .setPSetLayouts(mLayouts.data())
            .setPPushConstantRanges(mPushConstantRanges.data())
            .setPushConstantRangeCount((uint32_t)mPushConstantRanges.size());

    return device.createPipelineLayoutUnique(pipelineLayoutInfo);
}

GraphicsPipeline::Builder::Builder()
{
    mInputAssembly = vk::PipelineInputAssemblyStateCreateInfo()
            .setTopology(vk::PrimitiveTopology::eTriangleList);

    mRasterizationInfo = vk::PipelineRasterizationStateCreateInfo()
            .setLineWidth(1.0f)
            .setCullMode(vk::CullModeFlagBits::eBack)
            .setFrontFace(vk::FrontFace::eClockwise)
            .setPolygonMode(vk::PolygonMode::eFill);

    // TODO multisample as parameter
    mMultisampleInfo = vk::PipelineMultisampleStateCreateInfo()
            .setRasterizationSamples(vk::SampleCountFlagBits::e1)
            .setMinSampleShading(1.0f);
}

GraphicsPipeline::Builder& GraphicsPipeline::Builder::Shader(vk::ShaderModule shader,
                                                             vk::ShaderStageFlagBits shaderStage)
{
    auto shaderStageInfo = vk::PipelineShaderStageCreateInfo()
            .setModule(shader)
            .setPName("main")
            .setStage(shaderStage);

    mShaderStages.push_back(shaderStageInfo);

    return *this;
}

GraphicsPipeline::Builder& GraphicsPipeline::Builder::VertexAttribute(uint32_t location,
                                                                      uint32_t binding,
                                                                      vk::Format format,
                                                                      uint32_t offset)
{
    mVertexAttributeDescriptions.push_back({location, binding, format, offset});
    return *this;
}

GraphicsPipeline::Builder& GraphicsPipeline::Builder::VertexBinding(uint32_t binding,
                                                                    uint32_t stride,
                                                                    vk::VertexInputRate inputRate)
{
    mVertexBindingDescriptions.push_back({binding, stride, inputRate});
    return *this;
}

GraphicsPipeline::Builder& GraphicsPipeline::Builder::Topology(vk::PrimitiveTopology topology)
{
    mInputAssembly.setTopology(topology);
    return *this;
}

GraphicsPipeline::Builder& GraphicsPipeline::Builder::Layout(vk::PipelineLayout pipelineLayout)
{
    mPipelineLayout = pipelineLayout;
    return *this;
}

vk::UniquePipeline GraphicsPipeline::Builder::Create(vk::Device device, const RenderState& renderState)
{
    auto vertexInputInfo = vk::PipelineVertexInputStateCreateInfo()
            .setVertexBindingDescriptionCount((uint32_t)mVertexBindingDescriptions.size())
            .setPVertexBindingDescriptions(mVertexBindingDescriptions.data())
            .setVertexAttributeDescriptionCount((uint32_t)mVertexAttributeDescriptions.size())
            .setPVertexAttributeDescriptions(mVertexAttributeDescriptions.data());

    auto viewPort = vk::Viewport(0, 0, renderState.Width, renderState.Height, 0.0f, 1.0f);
    auto scissor = vk::Rect2D({0, 0}, {renderState.Width, renderState.Height});

    auto viewPortState = vk::PipelineViewportStateCreateInfo()
            .setScissorCount(1)
            .setPScissors(&scissor)
            .setViewportCount(1)
            .setPViewports(&viewPort);

    auto blendInfo = vk::PipelineColorBlendStateCreateInfo()
            .setAttachmentCount(1)
            .setPAttachments(&renderState.ColorBlend);

    auto pipelineInfo = vk::GraphicsPipelineCreateInfo()
            .setStageCount((uint32_t)mShaderStages.size())
            .setPStages(mShaderStages.data())
            .setPVertexInputState(&vertexInputInfo)
            .setPInputAssemblyState(&mInputAssembly)
            .setPRasterizationState(&mRasterizationInfo)
            .setPMultisampleState(&mMultisampleInfo)
            .setPColorBlendState(&blendInfo)
            .setLayout(mPipelineLayout)
            .setRenderPass(renderState.RenderPass)
            .setPViewportState(&viewPortState);

    return device.createGraphicsPipelineUnique(nullptr, pipelineInfo);
}

GraphicsPipeline::GraphicsPipeline()
{
}

GraphicsPipeline::GraphicsPipeline(GraphicsPipeline::Builder builder)
    : mBuilder(builder)
{
}

void GraphicsPipeline::Create(vk::Device device, const RenderState& renderState)
{
    auto it = std::find_if(mPipelines.begin(), mPipelines.end(), [&](const PipelineList::value_type& value)
    {
        return value.first == renderState;
    });

    if (it == mPipelines.end())
    {
        mPipelines.emplace_back(renderState, mBuilder.Create(device, renderState));
    }
}

void GraphicsPipeline::Bind(vk::CommandBuffer commandBuffer, const RenderState& renderState)
{
    auto it = std::find_if(mPipelines.begin(), mPipelines.end(), [&](const PipelineList::value_type& value)
    {
        return value.first == renderState;
    });

    if (it != mPipelines.end())
    {
        commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, *it->second);
    }
    else
    {
        throw std::runtime_error("No pipeline for this renderpass!");
    }
}

vk::UniquePipeline MakeComputePipeline(vk::Device device,
                                       vk::ShaderModule shader,
                                       vk::PipelineLayout layout,
                                       uint32_t localX,
                                       uint32_t localY)
{
    glm::uvec2 localSize(localX, localY);
    std::vector<vk::SpecializationMapEntry> mapEntries = {{1, offsetof(glm::uvec2, x), sizeof(glm::uvec2::x)},
                                                          {2, offsetof(glm::uvec2, y), sizeof(glm::uvec2::y)}};

    auto specialisationConst = vk::SpecializationInfo()
            .setMapEntryCount(mapEntries.size())
            .setPMapEntries(mapEntries.data())
            .setDataSize(sizeof(glm::uvec2))
            .setPData(&localSize);

    auto stageInfo = vk::PipelineShaderStageCreateInfo()
            .setModule(shader)
            .setPName("main")
            .setStage(vk::ShaderStageFlagBits::eCompute)
            .setPSpecializationInfo(&specialisationConst);

    auto pipelineInfo = vk::ComputePipelineCreateInfo()
            .setStage(stageInfo)
            .setLayout(layout);

    return device.createComputePipelineUnique(nullptr, pipelineInfo);
}

}}
