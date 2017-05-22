//
//  Pipeline.cpp
//  Vortex2D
//

#include "Pipeline.h"

#include <fstream>

namespace Vortex2D { namespace Renderer {

ShaderBuilder& ShaderBuilder::File(const std::string& fileName)
{
    std::ifstream is(fileName, std::ios::binary | std::ios::in | std::ios::ate);

    if (!is.is_open())
    {
        throw std::runtime_error("Couldn't open file:" + fileName);
    }

    size_t size = is.tellg();
    is.seekg(0, std::ios::beg);
    mContent.resize(size);
    is.read(mContent.data(), size);
    is.close();

    return *this;
}

vk::ShaderModule ShaderBuilder::Create(const Device& device)
{
    auto shaderInfo = vk::ShaderModuleCreateInfo()
            .setCodeSize(mContent.size())
            .setPCode((const uint32_t*)mContent.data());

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

// TODO is this format better? or like DescriptorSetLayout?
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

    // TODO blending as parameter
    mColorBlendAttachment = vk::PipelineColorBlendAttachmentState()
            .setColorWriteMask(vk::ColorComponentFlagBits::eR |
                               vk::ColorComponentFlagBits::eG |
                               vk::ColorComponentFlagBits::eB |
                               vk::ColorComponentFlagBits::eA)
            .setBlendEnable(false);
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

GraphicsPipeline::Builder& GraphicsPipeline::Builder::Layout(vk::PipelineLayout pipelineLayout)
{
    mPipelineLayout = pipelineLayout;
    return *this;
}

vk::UniquePipeline GraphicsPipeline::Builder::Create(vk::Device device,
                                                     uint32_t width,
                                                     uint32_t height,
                                                     vk::RenderPass renderPass)
{
    auto vertexInputInfo = vk::PipelineVertexInputStateCreateInfo()
            .setVertexBindingDescriptionCount(mVertexBindingDescriptions.size())
            .setPVertexBindingDescriptions(mVertexBindingDescriptions.data())
            .setVertexAttributeDescriptionCount(mVertexAttributeDescriptions.size())
            .setPVertexAttributeDescriptions(mVertexAttributeDescriptions.data());

    auto viewPort = vk::Viewport(0, 0, width, height, 0.0f, 1.0f);
    auto scissor = vk::Rect2D({0, 0}, {width, height});

    auto viewPortState = vk::PipelineViewportStateCreateInfo()
            .setScissorCount(1)
            .setPScissors(&scissor)
            .setViewportCount(1)
            .setPViewports(&viewPort);

    auto blendInfo = vk::PipelineColorBlendStateCreateInfo()
            .setAttachmentCount(1)
            .setPAttachments(&mColorBlendAttachment);

    auto pipelineInfo = vk::GraphicsPipelineCreateInfo()
            .setStageCount(mShaderStages.size())
            .setPStages(mShaderStages.data())
            .setPVertexInputState(&vertexInputInfo)
            .setPInputAssemblyState(&mInputAssembly)
            .setPRasterizationState(&mRasterizationInfo)
            .setPMultisampleState(&mMultisampleInfo)
            .setPColorBlendState(&blendInfo)
            .setLayout(mPipelineLayout)
            .setRenderPass(renderPass)
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

void GraphicsPipeline::Create(vk::Device device, uint32_t width, uint32_t height, vk::RenderPass renderPass)
{
    mPipelines[renderPass] = mBuilder.Create(device, width, height, renderPass);
}

void GraphicsPipeline::Bind(vk::CommandBuffer commandBuffer, vk::RenderPass renderPass)
{
    auto it = mPipelines.find(renderPass);
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
