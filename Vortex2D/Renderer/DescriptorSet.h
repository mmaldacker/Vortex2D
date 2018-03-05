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

struct ShaderLayout
{
    ShaderLayout(const SPIRV::Reflection& reflection);

    vk::ShaderStageFlags shaderStage;
    DescriptorTypeBindings bindings;
    unsigned pushConstantSize;
};

bool operator==(const ShaderLayout& left, const ShaderLayout& right);

struct PipelineLayout
{
    std::vector<ShaderLayout> layouts;
};

bool operator==(const PipelineLayout& left, const PipelineLayout& right);

struct DescriptorSet
{
    vk::UniqueDescriptorSet descriptorSet;
    vk::PipelineLayout pipelineLayout;
    vk::DescriptorSetLayout descriptorSetLayout;
};

class LayoutManager
{
public:
    LayoutManager(const Device& device);

    void CreateDescriptorPool();
    DescriptorSet MakeDescriptorSet(const PipelineLayout& layout);
    vk::DescriptorSetLayout GetDescriptorSetLayout(const PipelineLayout& layout);
    vk::PipelineLayout GetPipelineLayout(const PipelineLayout& layout);

private:
    const Device& mDevice;
    vk::UniqueDescriptorPool mDescriptorPool;
    std::vector<std::tuple<PipelineLayout, vk::UniqueDescriptorSetLayout>> mDescriptorSetLayouts;
    std::vector<std::tuple<PipelineLayout, vk::UniquePipelineLayout>> mPipelineLayouts;
};

struct DescriptorImage
{
    DescriptorImage(vk::Sampler sampler, Renderer::Texture& texture);
    DescriptorImage(Renderer::Texture& texture);

    vk::Sampler Sampler;
    Renderer::Texture* Texture;
};

struct BindingInput
{
    static constexpr uint32_t DefaultBind = static_cast<uint32_t>(-1);

    BindingInput(Renderer::GenericBuffer& buffer, uint32_t bind = DefaultBind);
    BindingInput(Renderer::Texture& texture,  uint32_t bind = DefaultBind);
    BindingInput(vk::Sampler sampler, Renderer::Texture& texture,  uint32_t bind = DefaultBind);

    uint32_t Bind;
    mpark::variant<Renderer::GenericBuffer*, DescriptorImage> Input;
};

void Bind(const Device& device, vk::DescriptorSet dstSet, const PipelineLayout& layout, const std::vector<BindingInput>& bindingInputs);

}}

#endif
