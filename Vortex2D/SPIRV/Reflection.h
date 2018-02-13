//
//  Reflection.h
//  Vortex2D
//

#ifndef Vortex2D_Reflection_h
#define Vortex2D_Reflection_h

#include <Vortex2D/Renderer/Common.h>
#include <Vortex2D/Renderer/DescriptorSet.h>
#include <Vortex2D/Renderer/Device.h>

#include <spirv_cross.hpp>
#include <map>

namespace Vortex2D { namespace SPIRV {

class Reflection
{
public:
  using DescriptorTypesMap = std::map<unsigned, vk::DescriptorType>;

  Reflection(const Renderer::SpirvBinary& spirv);

  DescriptorTypesMap GetDescriptorTypesMap() const;
  unsigned GetPushConstantsSize() const;

  vk::ShaderStageFlagBits GetShaderStage() const;

private:
  unsigned ReadBinding(unsigned id);

  spirv_cross::Compiler mCompiler;
  DescriptorTypesMap mDescriptorTypes;
};

}}

#endif

