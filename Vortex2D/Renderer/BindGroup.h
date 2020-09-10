//
//  BindGroup.h
//  Vortex2D
//

#ifndef Vortex2d_BindGroup_h
#define Vortex2d_BindGroup_h

#include <Vortex2D/Renderer/Buffer.h>
#include <Vortex2D/Renderer/Common.h>
#include <Vortex2D/Renderer/Texture.h>
#include <Vortex2D/SPIRV/ShaderLayout.h>

#include <Vortex2D/Utils/mapbox/variant.hpp>
#include <map>

namespace Vortex2D
{
namespace Renderer
{
class BindGroup
{
public:
  BindGroup();
  BindGroup(Handle::BindGroup bindGroup);
  BindGroup(Device& device, const Handle::BindGroupLayout& bindGroupLayout);
  BindGroup(BindGroup&& other);
  VORTEX2D_API ~BindGroup();

  BindGroup& operator=(BindGroup&& other);

  Handle::BindGroup Handle();

private:
  struct Impl;
  std::unique_ptr<Impl> mImpl;
};

/**
 * @brief The texture or sampler that can be bound to a shader
 */
struct Image
{
  Image(Renderer::Texture& texture, Sampler& sampler);
  Image(Renderer::Texture& texture);

  Renderer::Texture* Texture;
  Sampler* Sampler;
};

/**
 * @brief The texture/sampler or buffer that can be binded to a shader.
 */
struct BindingInput
{
  static constexpr uint32_t DefaultBind = static_cast<uint32_t>(-1);

  VORTEX2D_API BindingInput(Renderer::GenericBuffer& buffer, uint32_t bind = DefaultBind);
  VORTEX2D_API BindingInput(Renderer::Texture& texture, uint32_t bind = DefaultBind);
  VORTEX2D_API BindingInput(Sampler& sampler,
                            Renderer::Texture& texture,
                            uint32_t bind = DefaultBind);

  uint32_t Bind;
  mapbox::util::variant<Renderer::GenericBuffer*, Image> Input;
};

}  // namespace Renderer
}  // namespace Vortex2D

#endif
