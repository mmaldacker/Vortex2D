//
//  Pipeline.h
//  Vortex2D
//

#ifndef Vortex2d_Shader_h
#define Vortex2d_Shader_h

#include <Vortex2D/Renderer/Common.h>
#include <Vortex2D/Renderer/RenderState.h>

#include <string>
#include <vector>

namespace Vortex2D
{
namespace Renderer
{
class Device;

/**
 * @brief graphics pipeline which caches the pipeline per render states.
 */
class GraphicsPipeline
{
public:
  VORTEX2D_API GraphicsPipeline();

  /**
   * @brief Set the shader
   * @param shader the loaded shader
   * @param shaderStage shader state (vertex, fragment or compute)
   * @return *this
   */
  VORTEX2D_API GraphicsPipeline& Shader(vk::ShaderModule shader,
                                        vk::ShaderStageFlagBits shaderStage);

  /**
   * @brief Sets the vertex attributes
   * @param location location in the shader
   * @param binding binding in the shader
   * @param format vertex format
   * @param offset offset in the vertex
   * @return *this
   */
  VORTEX2D_API GraphicsPipeline& VertexAttribute(uint32_t location,
                                                 uint32_t binding,
                                                 vk::Format format,
                                                 uint32_t offset);

  /**
   * @brief Sets the vertex binding
   * @param binding binding in the shader
   * @param stride stride in bytes
   * @param inputRate inpute rate
   * @return *this
   */
  VORTEX2D_API GraphicsPipeline& VertexBinding(
      uint32_t binding,
      uint32_t stride,
      vk::VertexInputRate inputRate = vk::VertexInputRate::eVertex);

  VORTEX2D_API GraphicsPipeline& Topology(vk::PrimitiveTopology topology);
  VORTEX2D_API GraphicsPipeline& Layout(vk::PipelineLayout pipelineLayout);
  VORTEX2D_API GraphicsPipeline& DynamicState(vk::DynamicState dynamicState);

  friend class PipelineCache;
  friend bool operator==(const GraphicsPipeline&, const GraphicsPipeline&);
private:
  vk::PipelineMultisampleStateCreateInfo mMultisampleInfo;
  vk::PipelineRasterizationStateCreateInfo mRasterizationInfo;
  vk::PipelineInputAssemblyStateCreateInfo mInputAssembly;
  std::vector<vk::DynamicState> mDynamicStates;
  std::vector<vk::PipelineShaderStageCreateInfo> mShaderStages;
  std::vector<vk::VertexInputBindingDescription> mVertexBindingDescriptions;
  std::vector<vk::VertexInputAttributeDescription> mVertexAttributeDescriptions;
  vk::PipelineLayout mPipelineLayout;
};

bool operator==(const GraphicsPipeline& left, const GraphicsPipeline& right);

/**
 * @brief Defines and holds value of the specification constants for shaders
 */
struct SpecConstInfo
{
  template <typename Type>
  struct Value
  {
    uint32_t id;
    Type value;
  };

  vk::SpecializationInfo info;
  std::vector<vk::SpecializationMapEntry> mapEntries;
  std::vector<char> data;
};

bool operator==(const SpecConstInfo& left, const SpecConstInfo& right);

namespace Detail
{
inline void InsertSpecConst(SpecConstInfo& specConstInfo)
{
  specConstInfo.info.setMapEntryCount(static_cast<uint32_t>(specConstInfo.mapEntries.size()))
      .setPMapEntries(specConstInfo.mapEntries.data())
      .setDataSize(specConstInfo.data.size())
      .setPData(specConstInfo.data.data());
}

template <typename Arg, typename... Args>
inline void InsertSpecConst(SpecConstInfo& specConstInfo, Arg&& arg, Args&&... args)
{
  auto offset = static_cast<uint32_t>(specConstInfo.data.size());
  specConstInfo.data.resize(offset + sizeof(Arg));
  std::memcpy(&specConstInfo.data[offset], &arg.value, sizeof(Arg));
  specConstInfo.mapEntries.emplace_back(arg.id, offset, sizeof(Arg));

  InsertSpecConst(specConstInfo, std::forward<Args>(args)...);
}
}  // namespace Detail

/**
 * @brief Constructs a specialization constant value
 */
template <typename Type>
inline SpecConstInfo::Value<Type> SpecConstValue(uint32_t id, Type&& value)
{
  return SpecConstInfo::Value<Type>{id, value};
}

/**
 * @brief Constructs a @ref SpecConstInfo with given values of specialisation
 * constants.
 */
template <typename... Args>
inline SpecConstInfo SpecConst(Args&&... args)
{
  SpecConstInfo specConstInfo;
  Detail::InsertSpecConst(specConstInfo, std::forward<Args>(args)...);
  return specConstInfo;
}

/**
 * Create pipelines using vulkan's pipeline cache.
 */
class PipelineCache
{
public:
  PipelineCache(const Device& device);

  /**
   * @brief Create the pipeline cache.
   */
  void CreateCache();

  /**
   * @brief Create a graphics pipeline
   * @param builder
   * @param renderState
   * @return
   */
  VORTEX2D_API vk::Pipeline CreateGraphicsPipeline(const GraphicsPipeline& builder,
                                                   const RenderState& renderState);

  /**
   * @brief Create a compute pipeline
   * @param shader
   * @param layout
   * @param specConstInfo
   */
  VORTEX2D_API vk::Pipeline CreateComputePipeline(vk::ShaderModule shader,
                                                        vk::PipelineLayout layout,
                                                        SpecConstInfo specConstInfo = {});

private:
  struct GraphicsPipelineCache
  {
    RenderState RenderState;
    GraphicsPipeline GraphicsPipeline;
    vk::UniquePipeline Pipeline;
  };

  struct ComputePipelineCache
  {
    vk::ShaderModule Shader;
    vk::PipelineLayout Layout;
    SpecConstInfo SpecConstInfo;
    vk::UniquePipeline Pipeline;
  };

  const Device& mDevice;
  std::vector<GraphicsPipelineCache> mGraphicsPipelines;
  std::vector<ComputePipelineCache> mComputePipelines;
  vk::UniquePipelineCache mCache;
};

}  // namespace Renderer
}  // namespace Vortex2D

#endif
