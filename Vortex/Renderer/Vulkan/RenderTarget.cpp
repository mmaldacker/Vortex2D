//
//  RenderTarget.cpp
//  Vortex
//

#include <Vortex/Renderer/RenderTarget.h>

#include "Device.h"

#include <glm/gtc/matrix_transform.hpp>

namespace Vortex
{
namespace Renderer
{
struct RenderTarget::Impl
{
  Impl(Device& device, uint32_t width, uint32_t height, Handle::RenderPass renderPass)
      : mWidth(width)
      , mHeight(height)
      , mOrth(glm::ortho(0.0f, (float)width, 0.0f, (float)height))
      , mView(1.0f)
      , mRenderPass(reinterpret_cast<VkRenderPass>(renderPass),
                    static_cast<VulkanDevice&>(device).Handle())
  {
  }

  uint32_t GetWidth() const { return mWidth; }

  uint32_t GetHeight() const { return mHeight; }

  const glm::mat4& GetOrth() const { return mOrth; }

  const glm::mat4& GetView() const { return mView; }

  void SetView(const glm::mat4& view) { mView = view; }

  Handle::RenderPass GetRenderPass() const
  {
    VkRenderPass renderPass = *mRenderPass;
    return reinterpret_cast<Handle::RenderPass>(renderPass);
  }

  uint32_t mWidth;
  uint32_t mHeight;
  glm::mat4 mOrth;
  glm::mat4 mView;
  vk::UniqueRenderPass mRenderPass;
};

RenderTarget::RenderTarget(Device& device,
                           uint32_t width,
                           uint32_t height,
                           Handle::RenderPass renderPass)
    : mImpl(std::make_unique<Impl>(device, width, height, renderPass))
{
}

RenderTarget::RenderTarget(RenderTarget&& other) : mImpl(std::move(other.mImpl)) {}

RenderTarget::~RenderTarget() {}

uint32_t RenderTarget::GetWidth() const
{
  return mImpl->GetWidth();
}

uint32_t RenderTarget::GetHeight() const
{
  return mImpl->GetHeight();
}

const glm::mat4& RenderTarget::GetOrth() const
{
  return mImpl->GetOrth();
}

const glm::mat4& RenderTarget::GetView() const
{
  return mImpl->GetView();
}

void RenderTarget::SetView(const glm::mat4& view)
{
  mImpl->SetView(view);
}

Handle::RenderPass RenderTarget::GetRenderPass() const
{
  return mImpl->GetRenderPass();
}

}  // namespace Renderer
}  // namespace Vortex
