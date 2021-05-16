//
//  Reflection.h
//  Vortex2D
//

#ifndef Vortex2D_Reflection_h
#define Vortex2D_Reflection_h

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

  VORTEX2D_API Reflection(const Renderer::SpirvBinary& spirv);

  VORTEX2D_API DescriptorTypesMap GetDescriptorTypesMap() const;
  VORTEX2D_API unsigned GetPushConstantsSize() const;

  VORTEX2D_API vk::ShaderStageFlags GetShaderStage() const;

private:
  DescriptorTypesMap mDescriptorTypes;
  unsigned mPushConstantSize;
  vk::ShaderStageFlags mStageFlag;
};

}  // namespace SPIRV
}  // namespace Vortex2D

#endif
