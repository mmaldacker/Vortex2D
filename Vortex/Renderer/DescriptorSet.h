//
//  DescriptorSet.h
//  Vortex
//

#pragma once

#include <Vortex/Renderer/Buffer.h>
#include <Vortex/Renderer/Common.h>
#include <Vortex/Renderer/Texture.h>

#include <Vortex/Utils/mapbox/variant.hpp>
#include <map>

namespace Vortex
{
namespace SPIRV
{
class Reflection;
}

namespace Renderer
{
class Device;
using DescriptorTypeBindings = std::map<uint32_t, vk::DescriptorType>;

/**
 * @brief Represents the layout of a shader (vertex, fragment or compute)
 */
struct ShaderLayout
{
  VORTEX_API ShaderLayout(const SPIRV::Reflection& reflection);

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
  LayoutManager(Device& device);

  /**
   * @brief Create or re-create the descriptor pool, will render invalid
   * existing descriptor sets
   * @param size size of the pool
   */
  void CreateDescriptorPool(int size = 512);

  /**
   * @brief Create the descriptor set given the layout
   * @param layout pipeline/shader layout
   * @return built descriptor set
   */
  VORTEX_API DescriptorSet MakeDescriptorSet(const PipelineLayout& layout);

  /**
   * @brief Create, cache and return a descriptor layout given the pipeline
   * layout
   * @param layout pipeline layout
   * @return cached descriptor set layout
   */
  VORTEX_API vk::DescriptorSetLayout GetDescriptorSetLayout(const PipelineLayout& layout);

  /**
   * @brief create, cache and return a vulkan pipeline layout given the layout
   * @param layout pipeline layout
   * @return vulkan pipeline layout
   */
  VORTEX_API vk::PipelineLayout GetPipelineLayout(const PipelineLayout& layout);

private:
  Device& mDevice;
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

  VORTEX_API BindingInput(Renderer::GenericBuffer& buffer, uint32_t bind = DefaultBind);
  VORTEX_API BindingInput(Renderer::Texture& texture, uint32_t bind = DefaultBind);
  VORTEX_API BindingInput(vk::Sampler sampler,
                          Renderer::Texture& texture,
                          uint32_t bind = DefaultBind);

  uint32_t Bind;
  mapbox::util::variant<Renderer::GenericBuffer*, DescriptorImage> Input;
};

/**
 * @brief Bind the resources (buffer or texture/sampler) to a @ref DescriptorSet
 * @param device vulkan device
 * @param dstSet vulkan descriptor set
 * @param layout pipeline layout
 * @param bindingInputs list of resources (buffer or texture/sampler)
 */
VORTEX_API void Bind(Device& device,
                     DescriptorSet& dstSet,
                     const PipelineLayout& layout,
                     const std::vector<BindingInput>& bindingInputs);

}  // namespace Renderer
}  // namespace Vortex
