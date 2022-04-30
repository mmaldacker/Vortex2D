//
//  RenderWindow.cpp
//  Vortex2D
//

#include <Vortex/Renderer/RenderWindow.h>

#include <Vortex/Renderer/CommandBuffer.h>
#include <Vortex/Renderer/Drawable.h>

#include "Device.h"

namespace Vortex
{
namespace Renderer
{
struct RenderWindow::Impl
{
  Impl(RenderWindow& self, Device& device, Handle::Surface surface)
      : mSelf(self), mIndex(0), mFrameIndex(0)
  {
  }

  ~Impl() {}

  RenderCommand Record(RenderTarget::DrawableList drawables, ColorBlendState blendState)
  {
    return {};
  }

  void Submit(RenderCommand& renderCommand) { mRenderCommands.emplace_back(renderCommand); }

  void Display() {}

  RenderWindow& mSelf;
  std::vector<std::reference_wrapper<RenderCommand>> mRenderCommands;
  uint32_t mIndex;
  uint32_t mFrameIndex;
};

RenderWindow::RenderWindow(Device& device, Handle::Surface surface, uint32_t width, uint32_t height)
    : RenderTarget(device, width, height, {})  // TODO choose format
    , mImpl(std::make_unique<Impl>(*this, device, surface))
{
}

RenderWindow::~RenderWindow() {}

RenderCommand RenderWindow::Record(DrawableList drawables, ColorBlendState blendState)
{
  return mImpl->Record(drawables, blendState);
}

void RenderWindow::Submit(RenderCommand& renderCommand)
{
  mImpl->Submit(renderCommand);
}

void RenderWindow::Display()
{
  mImpl->Display();
}

}  // namespace Renderer
}  // namespace Vortex
