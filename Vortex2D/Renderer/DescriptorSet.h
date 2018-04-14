//
//  DescriptorSet.h
//  Vortex2D
//

#ifndef Vortex2d_DescriptorSet_h
#define Vortex2d_DescriptorSet_h

#include <Vortex2D/Renderer/Common.h>
#include <Vortex2D/Renderer/Buffer.h>
#include <Vortex2D/Renderer/Texture.h>

#include <Vortex2D/Utils/variant.hpp>
#include <map>

namespace Vortex2D {

namespace SPIRV {
class Reflection;
}

namespace Renderer {

class Device;
using DescriptorTypeBindings = std::map<uint32_t, vk::DescriptorType>;

/**
 * @brief Represents the layout of a shader (vertex, fragment or compute)
 */
struct ShaderLayout
{
    VORTEX2D_API ShaderLayout(const SPIRV::Reflection& reflection);

    vk::ShaderStageFlags shaderStage;
    DescriptorTypeBindings bindings;
    unsigned pushConstantSize;
};

bool operator==(const ShaderLayout& left, const ShaderLayout& right);

/**
 * @brief Represents the layout of a pipeline: vertex + fragment or compute
 */
struct PipelineLayout
{
    std::vector<ShaderLayout> layouts;
};

bool operator==(const PipelineLayout& left, const PipelineLayout& right);

/**
 * @brief The binding of an object for a shader.
 */
struct DescriptorSet
{
    vk::UniqueDescriptorSet descriptorSet;
    vk::PipelineLayout pipelineLayout;
    vk::DescriptorSetLayout descriptorSetLayout;
};

/**
 * @brief Caches and creates layouts and bindings
 */
class LayoutManager
{
public:
    VORTEX2D_API LayoutManager(const Device& device);

    /**
     * @brief Create or re-create the descriptor pool, will render invalid existing descriptor sets
     */
    void CreateDescriptorPool();

    /**
     * @brief Create the descriptor set given the layout
     * @param layout pipeline/shader layout
     * @return built descriptor set
     */
    VORTEX2D_API DescriptorSet MakeDescriptorSet(const PipelineLayout& layout);

    /**
     * @brief Create, cache and return a descriptor layout given the pipeline layout
     * @param layout pipeline layout
     * @return cached descriptor set layout
     */
    vk::DescriptorSetLayout GetDescriptorSetLayout(const PipelineLayout& layout);

    /**
     * @brief create, cache and return a vulkan pipeline layout given the layout
     * @param layout pipeline layout
     * @return vulkan pipeline layout
     */
    vk::PipelineLayout GetPipelineLayout(const PipelineLayout& layout);

private:
    const Device& mDevice;
    vk::UniqueDescriptorPool mDescriptorPool;
    std::vector<std::tuple<PipelineLayout, vk::UniqueDescriptorSetLayout>> mDescriptorSetLayouts;
    std::vector<std::tuple<PipelineLayout, vk::UniquePipelineLayout>> mPipelineLayouts;
};

/**
 * @brief The texture or sampler that can be binded to a shader
 */
struct DescriptorImage
{
    DescriptorImage(vk::Sampler sampler, Renderer::Texture& texture);
    DescriptorImage(Renderer::Texture& texture);

    vk::Sampler Sampler;
    Renderer::Texture* Texture;
};

/**
 * @brief The texture/sampler or buffer that can be binded to a shader.
 */
struct BindingInput
{
    static constexpr uint32_t DefaultBind = static_cast<uint32_t>(-1);

    VORTEX2D_API BindingInput(Renderer::GenericBuffer& buffer, uint32_t bind = DefaultBind);
    VORTEX2D_API BindingInput(Renderer::Texture& texture,  uint32_t bind = DefaultBind);
    VORTEX2D_API BindingInput(vk::Sampler sampler, Renderer::Texture& texture,  uint32_t bind = DefaultBind);

    uint32_t Bind;
    mpark::variant<Renderer::GenericBuffer*, DescriptorImage> Input;
};

/**
 * @brief Bind the resources (buffer or texture/sampler) to a @ref DescriptorSet
 * @param device vulkan device
 * @param dstSet vulkan descriptor set
 * @param layout pipeline layout
 * @param bindingInputs list of resources (buffer or texture/sampler)
 */
VORTEX2D_API void Bind(const Device& device, vk::DescriptorSet dstSet, const PipelineLayout& layout, const std::vector<BindingInput>& bindingInputs);

}}

#endif
