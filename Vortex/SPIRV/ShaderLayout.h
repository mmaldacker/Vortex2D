//
//  ShaderLayout.h
//  Vortex
//

#pragma once

#include <Vortex/Renderer/Common.h>
#include <Vortex/SPIRV/Reflection.h>
#include <map>

namespace Vortex
{
namespace SPIRV
{
using BindTypeBindings = std::map<uint32_t, Renderer::BindType>;

/**
 * @brief Represents the layout of a shader (vertex, fragment or compute)
 */
struct ShaderLayout
{
  VORTEX_API ShaderLayout();
  VORTEX_API ShaderLayout(const Reflection& reflection);

  Renderer::ShaderStage shaderStage;
  BindTypeBindings bindings;
  unsigned pushConstantSize;
};

bool operator==(const ShaderLayout& left, const ShaderLayout& right);

/**
 * @brief Represents the layout of a pipeline: vertex + fragment or compute
 */
using ShaderLayouts = std::vector<ShaderLayout>;

}  // namespace SPIRV
}  // namespace Vortex
