//
//  Reflection.h
//  Vortex
//

#pragma once

#include <Vortex/Renderer/Common.h>

#include <map>

namespace Vortex
{
namespace SPIRV
{
class Reflection
{
public:
  using DescriptorTypesMap = std::map<unsigned, Renderer::BindType>;

  VORTEX_API Reflection(const Renderer::SpirvBinary& spirv);

  VORTEX_API DescriptorTypesMap GetDescriptorTypesMap() const;
  VORTEX_API unsigned GetPushConstantsSize() const;

  VORTEX_API Renderer::ShaderStage GetShaderStage() const;

private:
  DescriptorTypesMap mDescriptorTypes;
  unsigned mPushConstantSize;
  Renderer::ShaderStage mStageFlag;
};

}  // namespace SPIRV
}  // namespace Vortex
