//
//  ShaderLayout.cpp
//  Vortex2D
//

#include "ShaderLayout.h"

namespace Vortex2D
{
namespace SPIRV
{
bool operator==(const ShaderLayout& left, const ShaderLayout& right)
{
  return left.bindings == right.bindings && left.pushConstantSize == right.pushConstantSize &&
         left.shaderStage == right.shaderStage;
}

ShaderLayout::ShaderLayout() {}

ShaderLayout::ShaderLayout(const SPIRV::Reflection& reflection)
    : shaderStage(reflection.GetShaderStage())
    , bindings(reflection.GetDescriptorTypesMap())
    , pushConstantSize(reflection.GetPushConstantsSize())
{
}

}  // namespace SPIRV
}  // namespace Vortex2D
