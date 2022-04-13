//
//  BindGroup.h
//  Vortex
//

#pragma once

#include <Vortex/Renderer/Buffer.h>
#include <Vortex/Renderer/Common.h>
#include <Vortex/Renderer/Texture.h>
#include <Vortex/SPIRV/ShaderLayout.h>

#include <Vortex/Utils/mapbox/variant.hpp>
#include <map>

namespace Vortex
{
namespace Renderer
{
class BindGroup
{
public:
  BindGroup();
  BindGroup(Device& device, const Handle::BindGroupLayout& bindGroupLayout);
  BindGroup(BindGroup&& other);
  VORTEX_API ~BindGroup();

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
  Image(Renderer::Texture& texture, Renderer::Sampler& sampler);
  Image(Renderer::Texture& texture);

  Renderer::Texture* Texture;
  Renderer::Sampler* Sampler;
};

/**
 * @brief The texture/sampler or buffer that can be binded to a shader.
 */
struct BindingInput
{
  static constexpr uint32_t DefaultBind = static_cast<uint32_t>(-1);

  VORTEX_API BindingInput(GenericBuffer& buffer, uint32_t bind = DefaultBind);
  VORTEX_API BindingInput(Texture& texture, uint32_t bind = DefaultBind);
  VORTEX_API BindingInput(Sampler& sampler, Texture& texture, uint32_t bind = DefaultBind);

  uint32_t Bind;
  mapbox::util::variant<GenericBuffer*, Image> Input;
};

}  // namespace Renderer
}  // namespace Vortex
