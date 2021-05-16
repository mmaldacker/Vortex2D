//
//  Reflection.h
//  Vortex
//

#pragma once

#include <Vortex2D/Renderer/Common.h>
#include <Vortex2D/Renderer/DescriptorSet.h>
#include <Vortex2D/Renderer/Device.h>

#include <map>

namespace Vortex
{
namespace SPIRV
{
class Reflection
{
public:
  using DescriptorTypesMap = std::map<unsigned, vk::DescriptorType>;

  VORTEX_API Reflection(const Renderer::SpirvBinary& spirv);

  VORTEX_API DescriptorTypesMap GetDescriptorTypesMap() const;
  VORTEX_API unsigned GetPushConstantsSize() const;

  VORTEX_API vk::ShaderStageFlags GetShaderStage() const;

private:
  DescriptorTypesMap mDescriptorTypes;
  unsigned mPushConstantSize;
  vk::ShaderStageFlags mStageFlag;
};

}  // namespace SPIRV
}  // namespace Vortex
