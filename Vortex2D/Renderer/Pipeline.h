//
//  Pipeline.h
//  Vortex2D
//

#ifndef Vortex2d_Shader_h
#define Vortex2d_Shader_h

#include <Vortex2D/Renderer/Common.h>
#include <Vortex2D/Renderer/Device.h>
#include <Vortex2D/Renderer/RenderState.h>

#include <string>
#include <vector>

namespace Vortex2D
{
namespace Renderer
{
/**
 * @brief graphics pipeline which caches the pipeline per render states.
 */
class GraphicsPipeline
{
public:
  /**
   * @brief Builder for graphics pipeline
   */
  class Builder
  {
  public:
    VORTEX2D_API Builder();

    /**
     * @brief Set the shader
     * @param shader the loaded shader
     * @param shaderStage shader state (vertex, fragment or compute)
     * @return *this
     */
    VORTEX2D_API Builder& Shader(vk::ShaderModule shader, vk::ShaderStageFlagBits shaderStage);

    /**
     * @brief Sets the vertex attributes
     * @param location location in the shader
     * @param binding binding in the shader
     * @param format vertex format
     * @param offset offset in the vertex
     * @return *this
     */
    VORTEX2D_API Builder& VertexAttribute(uint32_t location,
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
    VORTEX2D_API Builder& VertexBinding(
        uint32_t binding,
        uint32_t stride,
        vk::VertexInputRate inputRate = vk::VertexInputRate::eVertex);

    VORTEX2D_API Builder& Topology(vk::PrimitiveTopology topology);
    VORTEX2D_API Builder& Layout(vk::PipelineLayout pipelineLayout);
    VORTEX2D_API Builder& DynamicState(vk::DynamicState dynamicState);
    VORTEX2D_API vk::UniquePipeline Create(vk::Device device, const RenderState& renderState);

  private:
    vk::PipelineMultisampleStateCreateInfo mMultisampleInfo;
    vk::PipelineRasterizationStateCreateInfo mRasterizationInfo;
    vk::PipelineInputAssemblyStateCreateInfo mInputAssembly;
    std::vector<vk::DynamicState> mDynamicStates;
    std::vector<vk::PipelineShaderStageCreateInfo> mShaderStages;
    std::vector<vk::VertexInputBindingDescription> mVertexBindingDescriptions;
    std::vector<vk::VertexInputAttributeDescription> mVertexAttributeDescriptions;
    vk::PipelineLayout mPipelineLayout;
    vk::GraphicsPipelineCreateInfo mPipelineInfo;
  };

  VORTEX2D_API GraphicsPipeline();
  VORTEX2D_API GraphicsPipeline(Builder builder);

  VORTEX2D_API void Create(vk::Device device, const RenderState& renderState);
  VORTEX2D_API void Bind(vk::CommandBuffer commandBuffer, const RenderState& renderState);

private:
  Builder mBuilder;
  using PipelineList = std::vector<std::pair<RenderState, vk::UniquePipeline>>;
  PipelineList mPipelines;
};

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

  VORTEX2D_API SpecConstInfo();

  vk::SpecializationInfo info;
  std::vector<vk::SpecializationMapEntry> mapEntries;
  std::vector<char> data;
};

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
  auto offset = specConstInfo.data.size();
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
 * @brief Create a compute pipeline
 * @param device vulkan device
 * @param shader shader module
 * @param layout layout of shader
 * @param specConstInfo any specialisation constants
 * @return compute pipeline
 */
VORTEX2D_API vk::UniquePipeline MakeComputePipeline(vk::Device device,
                                                    vk::ShaderModule shader,
                                                    vk::PipelineLayout layout,
                                                    SpecConstInfo specConstInfo = {});

}  // namespace Renderer
}  // namespace Vortex2D

#endif
