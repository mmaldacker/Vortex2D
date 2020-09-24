//
//  ShaderLayout.h
//  Vortex2D
//

#ifndef Vortex2d_ShaderLayout_h
#define Vortex2d_ShaderLayout_h

#include <Vortex2D/Renderer/Common.h>
#include <Vortex2D/SPIRV/Reflection.h>
#include <map>

namespace Vortex2D
{
namespace SPIRV
{
using BindTypeBindings = std::map<uint32_t, Renderer::BindType>;

/**
 * @brief Represents the layout of a shader (vertex, fragment or compute)
 */
struct ShaderLayout
{
  VORTEX2D_API ShaderLayout();
  VORTEX2D_API ShaderLayout(const Reflection& reflection);

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
}  // namespace Vortex2D
#endif
