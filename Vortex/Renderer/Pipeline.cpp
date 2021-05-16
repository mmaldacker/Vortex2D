//
//  Pipeline.cpp
//  Vortex
//

#include "Pipeline.h"
#include <Vortex/Renderer/Device.h>
#include <algorithm>

namespace Vortex
{
namespace Renderer
{
GraphicsPipeline::GraphicsPipeline()
{
  mInputAssembly =
      vk::PipelineInputAssemblyStateCreateInfo().setTopology(vk::PrimitiveTopology::eTriangleList);

  mRasterizationInfo = vk::PipelineRasterizationStateCreateInfo()
                           .setLineWidth(1.0f)
                           .setCullMode(vk::CullModeFlagBits::eNone)
                           .setFrontFace(vk::FrontFace::eCounterClockwise)
                           .setPolygonMode(vk::PolygonMode::eFill);

  // TODO multisample as parameter
  mMultisampleInfo = vk::PipelineMultisampleStateCreateInfo()
                         .setRasterizationSamples(vk::SampleCountFlagBits::e1)
                         .setMinSampleShading(1.0f);
}

GraphicsPipeline& GraphicsPipeline::Shader(vk::ShaderModule shader,
                                           vk::ShaderStageFlagBits shaderStage)
{
  auto shaderStageInfo =
      vk::PipelineShaderStageCreateInfo().setModule(shader).setPName("main").setStage(shaderStage);

  mShaderStages.push_back(shaderStageInfo);

  return *this;
}

GraphicsPipeline& GraphicsPipeline::VertexAttribute(uint32_t location,
                                                    uint32_t binding,
                                                    vk::Format format,
                                                    uint32_t offset)
{
  mVertexAttributeDescriptions.push_back({location, binding, format, offset});
  return *this;
}

GraphicsPipeline& GraphicsPipeline::VertexBinding(uint32_t binding,
                                                  uint32_t stride,
                                                  vk::VertexInputRate inputRate)
{
  mVertexBindingDescriptions.push_back({binding, stride, inputRate});
  return *this;
}

GraphicsPipeline& GraphicsPipeline::Topology(vk::PrimitiveTopology topology)
{
  mInputAssembly.setTopology(topology);
  return *this;
}

GraphicsPipeline& GraphicsPipeline::Layout(vk::PipelineLayout pipelineLayout)
{
  mPipelineLayout = pipelineLayout;
  return *this;
}

GraphicsPipeline& GraphicsPipeline::DynamicState(vk::DynamicState dynamicState)
{
  mDynamicStates.push_back(dynamicState);
  return *this;
}

bool operator==(const GraphicsPipeline& left, const GraphicsPipeline& right)
{
  return left.mDynamicStates == right.mDynamicStates &&
         left.mInputAssembly == right.mInputAssembly &&
         left.mPipelineLayout == right.mPipelineLayout &&
         left.mMultisampleInfo == right.mMultisampleInfo &&
         left.mRasterizationInfo == right.mRasterizationInfo &&
         left.mVertexBindingDescriptions == right.mVertexBindingDescriptions &&
         left.mVertexAttributeDescriptions == right.mVertexAttributeDescriptions &&
         left.mShaderStages == right.mShaderStages;
}

SpecConstInfo::SpecConstInfo() {}

bool operator==(const SpecConstInfo& left, const SpecConstInfo& right)
{
  return left.data == right.data && left.mapEntries == right.mapEntries;
}

PipelineCache::PipelineCache(const Device& device) : mDevice(device) {}

void PipelineCache::CreateCache()
{
  auto info = vk::PipelineCacheCreateInfo();
  mCache = mDevice.Handle().createPipelineCacheUnique(info);
}

vk::Pipeline PipelineCache::CreateGraphicsPipeline(const GraphicsPipeline& graphics,
                                                   const RenderState& renderState)
{
  auto it = std::find_if(mGraphicsPipelines.begin(),
                         mGraphicsPipelines.end(),
                         [&](const GraphicsPipelineCache& pipeline) {
                           return pipeline.Graphics == graphics && pipeline.State == renderState;
                         });

  if (it != mGraphicsPipelines.end())
  {
    return *it->Pipeline;
  }

  auto vertexInputInfo =
      vk::PipelineVertexInputStateCreateInfo()
          .setVertexBindingDescriptionCount((uint32_t)graphics.mVertexBindingDescriptions.size())
          .setPVertexBindingDescriptions(graphics.mVertexBindingDescriptions.data())
          .setVertexAttributeDescriptionCount(
              (uint32_t)graphics.mVertexAttributeDescriptions.size())
          .setPVertexAttributeDescriptions(graphics.mVertexAttributeDescriptions.data());

  auto viewPort = vk::Viewport(0,
                               0,
                               static_cast<float>(renderState.Width),
                               static_cast<float>(renderState.Height),
                               0.0f,
                               1.0f);
  auto scissor = vk::Rect2D({0, 0}, {renderState.Width, renderState.Height});

  auto viewPortState = vk::PipelineViewportStateCreateInfo()
                           .setScissorCount(1)
                           .setPScissors(&scissor)
                           .setViewportCount(1)
                           .setPViewports(&viewPort);

  auto blendInfo = vk::PipelineColorBlendStateCreateInfo()
                       .setAttachmentCount(1)
                       .setPAttachments(&renderState.BlendState.ColorBlend)
                       .setBlendConstants(renderState.BlendState.BlendConstants);

  auto dynamicState = vk::PipelineDynamicStateCreateInfo()
                          .setPDynamicStates(graphics.mDynamicStates.data())
                          .setDynamicStateCount((uint32_t)graphics.mDynamicStates.size());

  auto pipelineInfo = vk::GraphicsPipelineCreateInfo()
                          .setStageCount((uint32_t)graphics.mShaderStages.size())
                          .setPStages(graphics.mShaderStages.data())
                          .setPVertexInputState(&vertexInputInfo)
                          .setPInputAssemblyState(&graphics.mInputAssembly)
                          .setPRasterizationState(&graphics.mRasterizationInfo)
                          .setPMultisampleState(&graphics.mMultisampleInfo)
                          .setPColorBlendState(&blendInfo)
                          .setLayout(graphics.mPipelineLayout)
                          .setRenderPass(renderState.RenderPass)
                          .setPViewportState(&viewPortState)
                          .setPDynamicState(&dynamicState);

  GraphicsPipelineCache pipeline = {
      renderState, graphics, mDevice.Handle().createGraphicsPipelineUnique(*mCache, pipelineInfo)};
  mGraphicsPipelines.push_back(std::move(pipeline));
  return *mGraphicsPipelines.back().Pipeline;
}

vk::Pipeline PipelineCache::CreateComputePipeline(vk::ShaderModule shader,
                                                  vk::PipelineLayout layout,
                                                  SpecConstInfo specConstInfo)
{
  auto it = std::find_if(mComputePipelines.begin(),
                         mComputePipelines.end(),
                         [&](const ComputePipelineCache& pipeline) {
                           return pipeline.Shader == shader && pipeline.Layout == layout &&
                                  pipeline.SpecConst == specConstInfo;
                         });

  if (it != mComputePipelines.end())
  {
    return *it->Pipeline;
  }

  auto stageInfo = vk::PipelineShaderStageCreateInfo()
                       .setModule(shader)
                       .setPName("main")
                       .setStage(vk::ShaderStageFlagBits::eCompute)
                       .setPSpecializationInfo(&specConstInfo.info);

  auto pipelineInfo = vk::ComputePipelineCreateInfo().setStage(stageInfo).setLayout(layout);

  mComputePipelines.push_back(
      {shader,
       layout,
       specConstInfo,
       mDevice.Handle().createComputePipelineUnique(*mCache, pipelineInfo)});
  return *mComputePipelines.back().Pipeline;
}

}  // namespace Renderer
}  // namespace Vortex
