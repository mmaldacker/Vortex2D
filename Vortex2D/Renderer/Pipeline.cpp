//
//  Pipeline.cpp
//  Vortex2D
//

#include "Pipeline.h"

#include <fstream>
#include <algorithm>

namespace Vortex2D { namespace Renderer {


vk::ShaderModule MakeShader(const Device& device, const std::string& filename)
{

    std::ifstream is(filename, std::ios::binary | std::ios::in | std::ios::ate);

    if (!is.is_open())
    {
        throw std::runtime_error("Couldn't open file:" + filename);
    }

    std::vector<char> content;

    size_t size = is.tellg();
    is.seekg(0, std::ios::beg);
    content.resize(size);
    is.read(content.data(), size);
    is.close();

    auto shaderInfo = vk::ShaderModuleCreateInfo()
            .setCodeSize(content.size())
            .setPCode((const uint32_t*)content.data());

    return device.CreateShaderModule(shaderInfo);
}

PipelineLayout& PipelineLayout::DescriptorSetLayout(vk::DescriptorSetLayout layout)
{
    mLayouts.push_back(layout);
    return *this;
}

PipelineLayout PipelineLayout::Create(vk::Device device)
{
    auto pipelineLayoutInfo = vk::PipelineLayoutCreateInfo()
            .setSetLayoutCount(mLayouts.size())
            .setPSetLayouts(mLayouts.data());
    mPipelineLayout = device.createPipelineLayoutUnique(pipelineLayoutInfo);
    return std::move(*this);
}

PipelineLayout::operator vk::PipelineLayout() const
{
    return *mPipelineLayout;
}

GraphicsPipeline::Builder::Builder()
{
    // TODO topology as parameter
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
            .setVertexBindingDescriptionCount(mVertexBindingDescriptions.size())
            .setPVertexBindingDescriptions(mVertexBindingDescriptions.data())
            .setVertexAttributeDescriptionCount(mVertexAttributeDescriptions.size())
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
            .setStageCount(mShaderStages.size())
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
    mPipelines.emplace_back(renderState, mBuilder.Create(device, renderState));
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


ComputePipelineBuilder& ComputePipelineBuilder::Shader(vk::ShaderModule shader)
{
    mStageInfo
            .setModule(shader)
            .setPName("main")
            .setStage(vk::ShaderStageFlagBits::eCompute);

    mPipelineInfo.setStage(mStageInfo);
    return *this;
}

ComputePipelineBuilder& ComputePipelineBuilder::Layout(vk::PipelineLayout layout)
{
    mPipelineInfo.setLayout(layout);
    return *this;
}

vk::UniquePipeline ComputePipelineBuilder::Create(vk::Device device)
{
    return device.createComputePipelineUnique(nullptr, mPipelineInfo);
}

}}
