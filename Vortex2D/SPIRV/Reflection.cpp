//
//  Reflection.cpp
//  Vortex2D
//

#include "Reflection.h"

namespace Vortex2D { namespace SPIRV {

Reflection::Reflection(const Renderer::SpirvBinary& spirv)
  : mCompiler(spirv.data(), spirv.words())
{
    const auto& resources = mCompiler.get_shader_resources();

    // read storage buffers
    for (auto& resource: resources.storage_buffers)
    {
        mDescriptorTypes[ReadBinding(resource.id)] = vk::DescriptorType::eStorageBuffer;
    }

    // read storage images
    for (auto& resource: resources.storage_images)
    {
        mDescriptorTypes[ReadBinding(resource.id)] = vk::DescriptorType::eStorageImage;
    }

    // read combined samplers
    for (auto& resource: resources.sampled_images)
    {
        mDescriptorTypes[ReadBinding(resource.id)] = vk::DescriptorType::eCombinedImageSampler;
    }

    // read uniforms
    for (auto& resource: resources.uniform_buffers)
    {
        mDescriptorTypes[ReadBinding(resource.id)] = vk::DescriptorType::eUniformBuffer;
    }
}

unsigned Reflection::ReadBinding(unsigned id)
{
    unsigned set = mCompiler.get_decoration(id, spv::DecorationDescriptorSet);
    if (set != 0) throw std::runtime_error("Currently only support set = 0");
    unsigned binding = mCompiler.get_decoration(id, spv::DecorationBinding);
    if (mDescriptorTypes.count(binding)) throw std::runtime_error("Duplicate binding");
    return binding;
}

Reflection::DescriptorTypesMap Reflection::GetDescriptorTypesMap() const
{
    return mDescriptorTypes;
}

unsigned Reflection::GetPushConstantsSize() const
{
    const auto& resources = mCompiler.get_shader_resources();
    if (resources.push_constant_buffers.empty())
    {
        return 0;
    }

    assert(resources.push_constant_buffers.size() == 1);

    unsigned id = resources.push_constant_buffers[0].id;
    auto type = mCompiler.get_type_from_variable(id);
    return static_cast<unsigned>(mCompiler.get_declared_struct_size(type));
}

vk::ShaderStageFlagBits Reflection::GetShaderStage() const
{
    switch (mCompiler.get_execution_model())
    {
        case spv::ExecutionModelVertex:
            return vk::ShaderStageFlagBits::eVertex;
        case spv::ExecutionModelFragment:
            return vk::ShaderStageFlagBits::eFragment;
        case spv::ExecutionModelGLCompute:
            return vk::ShaderStageFlagBits::eCompute;
        default:
            throw std::runtime_error("unsupported execution model");
    }
}

}}


