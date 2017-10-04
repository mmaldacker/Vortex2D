//
//  Reflection.cpp
//  Vortex2D
//

#include "Reflection.h"

namespace Vortex2D { namespace SPIRV {


Reflection::Reflection(const std::vector<uint32_t>& spirv)
    : mCompiler(spirv)
{

}

unsigned Reflection::ReadBinding(unsigned id)
{
    unsigned set = mCompiler.get_decoration(id, spv::DecorationDescriptorSet);
    if (set != 0) throw std::runtime_error("Currently only support set = 0");
    unsigned binding = mCompiler.get_decoration(id, spv::DecorationBinding);
    if (mDescriptorTypes.count(binding)) throw std::runtime_error("Duplicate binding");
    return binding;
}

std::vector<vk::DescriptorType> Reflection::GetDescriptorTypes()
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

    // create vector
    std::vector<vk::DescriptorType> descriptorTypes;
    int i = 0;
    for (auto& descriptorTypePair: mDescriptorTypes)
    {
        if (descriptorTypePair.first != i) throw std::runtime_error("Missing binding");
        descriptorTypes.push_back(descriptorTypePair.second);
        i++;
    }

    return descriptorTypes;
}

unsigned Reflection::GetPushConstantsSize()
{
    const auto& resources = mCompiler.get_shader_resources();
    assert(resources.push_constant_buffers.size() == 1);

    unsigned id = resources.push_constant_buffers[0].id;
    auto type = mCompiler.get_type_from_variable(id);
    return mCompiler.get_declared_struct_size(type);
}

}}


