//
//  Reflection.h
//  Vortex2D
//

#ifndef Vortex2D_Reflection_h
#define Vortex2D_Reflection_h

#include <Vortex2D/Renderer/Common.h>
#include <spirv_cross.hpp>
#include <map>

namespace Vortex2D { namespace SPIRV {

class Reflection
{
public:
  Reflection(const std::vector<uint32_t>& spirv);
  
  // TODO read push constants too

  std::vector<vk::DescriptorType> GetDescriptorTypes();
  unsigned GetPushConstantsSize();
  
private:
  unsigned ReadBinding(unsigned id);

  spirv_cross::Compiler mCompiler;
  std::map<unsigned, vk::DescriptorType> mDescriptorTypes;
};

}}

#endif

