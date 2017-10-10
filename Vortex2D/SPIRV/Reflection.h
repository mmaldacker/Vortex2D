//
//  Reflection.h
//  Vortex2D
//

#ifndef Vortex2D_Reflection_h
#define Vortex2D_Reflection_h

#include <Vortex2D/Renderer/Common.h>
#include <Vortex2D/Renderer/DescriptorSet.h>

#include <spirv_cross.hpp>
#include <map>

namespace Vortex2D { namespace SPIRV {

class Reflection
{
public:
  using DescriptorTypesMap = std::map<unsigned, vk::DescriptorType>;

  Reflection(const std::vector<uint32_t>& spirv);

  // TODO read push constants too

  std::vector<vk::DescriptorType> GetDescriptorTypes() const;
  DescriptorTypesMap GetDescriptorTypesMap() const;
  unsigned GetPushConstantsSize();

  vk::ShaderStageFlagBits GetShaderStage();

  void AddBindings(Renderer::DescriptorSetLayoutBuilder& builder);

private:
  unsigned ReadBinding(unsigned id);

  spirv_cross::Compiler mCompiler;
  DescriptorTypesMap mDescriptorTypes;
};

}}

#endif

