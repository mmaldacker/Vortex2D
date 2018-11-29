//
//  Pipeline.h
//  Vortex2D
//

#ifndef Vortex2d_Shader_h
#define Vortex2d_Shader_h

#include <Vortex2D/Renderer/Common.h>
#include <Vortex2D/Renderer/Device.h>
#include <Vortex2D/Renderer/RenderState.h>

#include <vector>
#include <string>

namespace Vortex2D { namespace Renderer {

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
        Builder();

        /**
         * @brief Set the shader
         * @param shader the loaded shader
         * @param shaderStage shader state (vertex, fragment or compute)
         * @return *this
         */
        Builder& Shader(vk::ShaderModule shader, vk::ShaderStageFlagBits shaderStage);

        /**
         * @brief Sets the vertex attributes
         * @param location location in the shader
         * @param binding binding in the shader
         * @param format vertex format
         * @param offset offset in the vertex
         * @return *this
         */
        Builder& VertexAttribute(uint32_t location, uint32_t binding, vk::Format format, uint32_t offset);

        /**
         * @brief Sets the vertex binding
         * @param binding binding in the shader
         * @param stride stride in bytes
         * @param inputRate inpute rate
         * @return *this
         */
        Builder& VertexBinding(uint32_t binding, uint32_t stride, vk::VertexInputRate inputRate = vk::VertexInputRate::eVertex);

        Builder& Topology(vk::PrimitiveTopology topology);
        Builder& Layout(vk::PipelineLayout pipelineLayout);
        vk::UniquePipeline Create(vk::Device device, const RenderState& renderState);

    private:
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

    void Create(vk::Device device, const RenderState& renderState);
    void Bind(vk::CommandBuffer commandBuffer, const RenderState& renderState);

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
    vk::SpecializationInfo info;
    std::vector<vk::SpecializationMapEntry> mapEntries;
    std::vector<char> data;
};

namespace Detail
{
inline void InsertSpecConst(SpecConstInfo& , uint32_t , uint32_t )
{

}

template<typename Arg, typename... Args>
inline void InsertSpecConst(SpecConstInfo& specConstInfo,
                     uint32_t id,
                     uint32_t offset,
                     Arg&& arg,
                     Args&&... args)
{
    specConstInfo.data.resize(offset + sizeof(Arg));
    std::memcpy(&specConstInfo.data[offset], &arg, sizeof(Arg));
    specConstInfo.mapEntries.emplace_back(id, offset, sizeof(Arg));

    InsertSpecConst(specConstInfo, id + 1, offset + sizeof(Arg), std::forward<Args>(args)...);
}
}

/**
 * @brief Constructs a @ref SpecConstInfo with given values of specialisation constants.
 */
template<typename...Args>
inline SpecConstInfo SpecConst(Args&&... args)
{
    SpecConstInfo specConstInfo;

    Detail::InsertSpecConst(specConstInfo, 0, 0, std::forward<Args>(args)...);

    specConstInfo.info
            .setMapEntryCount(static_cast<uint32_t>(specConstInfo.mapEntries.size()))
            .setPMapEntries(specConstInfo.mapEntries.data())
            .setDataSize(specConstInfo.data.size())
            .setPData(specConstInfo.data.data());

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

}}

#endif
