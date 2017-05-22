//
//  Pipeline.h
//  Vortex2D
//

#ifndef Vortex_Shader_h
#define Vortex_Shader_h

#include <Vortex2D/Renderer/Common.h>
#include <Vortex2D/Renderer/Device.h>

#include <map>
#include <string>

namespace Vortex2D { namespace Renderer {

class ShaderBuilder
{
public:
    ShaderBuilder& File(const std::string& fileName);

    vk::ShaderModule Create(const Device& device);

private:
    std::vector<char> mContent;
};

class PipelineLayout
{
public:
    PipelineLayout& DescriptorSetLayout(vk::DescriptorSetLayout layout);
    PipelineLayout Create(vk::Device device);
    operator vk::PipelineLayout() const;

private:
    std::vector<vk::DescriptorSetLayout> mLayouts;
    vk::UniquePipelineLayout mPipelineLayout;
};

class GraphicsPipeline
{
public:
    // TODO is this format better? or like DescriptorSetLayout?
    class Builder
    {
    public:
        Builder();

        Builder& Shader(vk::ShaderModule shader, vk::ShaderStageFlagBits shaderStage);
        Builder& VertexAttribute(uint32_t location, uint32_t binding, vk::Format format, uint32_t offset);
        Builder& VertexBinding(uint32_t binding, uint32_t stride, vk::VertexInputRate inputRate = vk::VertexInputRate::eVertex);

        Builder& Layout(vk::PipelineLayout pipelineLayout);
        vk::UniquePipeline Create(vk::Device device, uint32_t width, uint32_t height, vk::RenderPass renderPass);
    private:
        vk::PipelineColorBlendAttachmentState mColorBlendAttachment;
        vk::PipelineMultisampleStateCreateInfo mMultisampleInfo;
        vk::PipelineRasterizationStateCreateInfo mRasterizationInfo;
        vk::PipelineInputAssemblyStateCreateInfo mInputAssembly;
        std::vector<vk::PipelineShaderStageCreateInfo> mShaderStages;
        std::vector<vk::VertexInputBindingDescription> mVertexBindingDescriptions;
        std::vector<vk::VertexInputAttributeDescription> mVertexAttributeDescriptions;
        vk::PipelineLayout mPipelineLayout;
        vk::GraphicsPipelineCreateInfo mPipelineInfo;
    };

    GraphicsPipeline();
    GraphicsPipeline(Builder builder);

    void Create(vk::Device device, uint32_t width, uint32_t height, vk::RenderPass renderPass);
    void Bind(vk::CommandBuffer commandBuffer, vk::RenderPass renderPass);

private:
    Builder mBuilder;
    std::map<vk::RenderPass, vk::UniquePipeline> mPipelines;
};

class ComputePipelineBuilder
{
public:
    ComputePipelineBuilder& Shader(vk::ShaderModule shader);
    ComputePipelineBuilder& Layout(vk::PipelineLayout layout);
    vk::UniquePipeline Create(vk::Device device);

private:
    vk::ComputePipelineCreateInfo mPipelineInfo;
    vk::PipelineLayout mLayout;
    vk::PipelineShaderStageCreateInfo mStageInfo;
};

}}

#endif
