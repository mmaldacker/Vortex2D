//
//  Reflection.cpp
//  Vortex2D
//

#include "Reflection.h"
#include <spirv_cross.hpp>

namespace Vortex2D
{
namespace SPIRV
{
unsigned ReadBinding(spirv_cross::Compiler& compiler,
                     Reflection::DescriptorTypesMap& descriptorType,
                     unsigned id)
{
  unsigned set = compiler.get_decoration(id, spv::DecorationDescriptorSet);
  if (set != 0)
    throw std::runtime_error("Currently only support set = 0");
  unsigned binding = compiler.get_decoration(id, spv::DecorationBinding);
  if (descriptorType.count(binding))
    throw std::runtime_error("Duplicate binding");
  return binding;
}

Reflection::Reflection(const Renderer::SpirvBinary& spirv) : mPushConstantSize(0)
{
  spirv_cross::Compiler compiler(spirv.data(), spirv.words());
  const auto& resources = compiler.get_shader_resources();

  // read storage buffers
  for (auto& resource : resources.storage_buffers)
  {
    mDescriptorTypes[ReadBinding(compiler, mDescriptorTypes, resource.id)] =
        vk::DescriptorType::eStorageBuffer;
  }

  // read storage images
  for (auto& resource : resources.storage_images)
  {
    mDescriptorTypes[ReadBinding(compiler, mDescriptorTypes, resource.id)] =
        vk::DescriptorType::eStorageImage;
  }

  // read combined samplers
  for (auto& resource : resources.sampled_images)
  {
    mDescriptorTypes[ReadBinding(compiler, mDescriptorTypes, resource.id)] =
        vk::DescriptorType::eCombinedImageSampler;
  }

  // read uniforms
  for (auto& resource : resources.uniform_buffers)
  {
    mDescriptorTypes[ReadBinding(compiler, mDescriptorTypes, resource.id)] =
        vk::DescriptorType::eUniformBuffer;
  }

  // get push constant size
  if (!resources.push_constant_buffers.empty())
  {
    assert(resources.push_constant_buffers.size() == 1);

    unsigned id = resources.push_constant_buffers[0].id;
    auto type = compiler.get_type_from_variable(id);
    mPushConstantSize = static_cast<unsigned>(compiler.get_declared_struct_size(type));
  }

  // get shader type
  switch (compiler.get_execution_model())
  {
    case spv::ExecutionModelVertex:
      mStageFlag = vk::ShaderStageFlagBits::eVertex;
      break;
    case spv::ExecutionModelFragment:
      mStageFlag = vk::ShaderStageFlagBits::eFragment;
      break;
    case spv::ExecutionModelGLCompute:
      mStageFlag = vk::ShaderStageFlagBits::eCompute;
      break;
    default:
      throw std::runtime_error("unsupported execution model");
  }
}

Reflection::DescriptorTypesMap Reflection::GetDescriptorTypesMap() const
{
  return mDescriptorTypes;
}

unsigned Reflection::GetPushConstantsSize() const
{
  return mPushConstantSize;
}

vk::ShaderStageFlags Reflection::GetShaderStage() const
{
  return mStageFlag;
}

}  // namespace SPIRV
}  // namespace Vortex2D
