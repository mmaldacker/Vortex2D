//
//  RenderTexture.cpp
//  Vortex
//

#include <Vortex/Renderer/RenderTexture.h>

#include <Vortex/Renderer/CommandBuffer.h>
#include <Vortex/Renderer/Drawable.h>

#include "Device.h"

namespace Vortex
{
namespace Renderer
{
struct RenderTexture::Impl
{
  Impl(RenderTexture& self, Device& device) : mSelf(self) {}

  RenderCommand Record(DrawableList drawables, ColorBlendState blendState) { return {}; }

  void Submit(RenderCommand& renderCommand) { renderCommand.Render(); }

  RenderTexture& mSelf;
};

RenderTexture::RenderTexture(Device& device, uint32_t width, uint32_t height, Format format)
    : RenderTarget(device, width, height, {})
    , Texture(device, width, height, format)
    , mImpl(std::make_unique<Impl>(*this, device))
{
}

RenderTexture::RenderTexture(RenderTexture&& other)
    : RenderTarget(std::move(other)), Texture(std::move(other)), mImpl(std::move(other.mImpl))
{
}

RenderTexture::~RenderTexture() {}

RenderCommand RenderTexture::Record(DrawableList drawables, ColorBlendState blendState)
{
  return mImpl->Record(drawables, blendState);
}

void RenderTexture::Submit(RenderCommand& renderCommand)
{
  mImpl->Submit(renderCommand);
}

}  // namespace Renderer
}  // namespace Vortex
