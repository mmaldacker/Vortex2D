//
//  Pipeline.cpp
//  Vortex
//

#include "Pipeline.h"
#include <Vortex/Renderer/Device.h>
#include <Vortex/Renderer/Vulkan/Vulkan.h>
#include <algorithm>

namespace Vortex
{
namespace Renderer
{
GraphicsPipelineDescriptor::GraphicsPipelineDescriptor() {}

GraphicsPipelineDescriptor& GraphicsPipelineDescriptor::Shader(Handle::ShaderModule shader,
                                                               ShaderStage shaderStage)
{
  shaders.push_back({shader, shaderStage});
  return *this;
}

GraphicsPipelineDescriptor& GraphicsPipelineDescriptor::VertexAttribute(uint32_t location,
                                                                        uint32_t binding,
                                                                        Format format,
                                                                        uint32_t offset)
{
  vertexAttributes.push_back({location, binding, format, offset});
  return *this;
}

GraphicsPipelineDescriptor& GraphicsPipelineDescriptor::VertexBinding(uint32_t binding,
                                                                      uint32_t stride)
{
  vertexBindings.push_back({binding, stride});
  return *this;
}

GraphicsPipelineDescriptor& GraphicsPipelineDescriptor::Topology(PrimitiveTopology topology)
{
  primitiveTopology = topology;
  return *this;
}

GraphicsPipelineDescriptor& GraphicsPipelineDescriptor::Layout(Handle::PipelineLayout layout)
{
  pipelineLayout = layout;
  return *this;
}

bool operator==(const GraphicsPipelineDescriptor::ShaderDescriptor& left,
                const GraphicsPipelineDescriptor::ShaderDescriptor& right)
{
  return left.shaderStage == right.shaderStage && left.shaderModule == right.shaderModule;
}

bool operator==(const GraphicsPipelineDescriptor::VertexBindingDescriptor& left,
                const GraphicsPipelineDescriptor::VertexBindingDescriptor& right)
{
  return left.stride == right.stride && left.binding == right.binding;
}

bool operator==(const GraphicsPipelineDescriptor::VertexAttributeDescriptor& left,
                const GraphicsPipelineDescriptor::VertexAttributeDescriptor& right)
{
  return left.format == right.format && left.offset == right.offset &&
         left.binding == right.binding && left.location == right.location;
}

bool operator==(const GraphicsPipelineDescriptor& left, const GraphicsPipelineDescriptor& right)
{
  return left.shaders == right.shaders && left.vertexAttributes == right.vertexAttributes &&
         left.vertexBindings == right.vertexBindings &&
         left.primitiveTopology == right.primitiveTopology &&
         left.pipelineLayout == right.pipelineLayout;
}

SpecConstInfo::SpecConstInfo() {}

bool operator==(const SpecConstInfo::Entry& left, const SpecConstInfo::Entry& right)
{
  return left.size == right.size && left.offset == right.offset &&
         left.constantID == right.constantID;
}

bool operator==(const SpecConstInfo& left, const SpecConstInfo& right)
{
  return left.data == right.data && left.mapEntries == right.mapEntries;
}

}  // namespace Renderer
}  // namespace Vortex
