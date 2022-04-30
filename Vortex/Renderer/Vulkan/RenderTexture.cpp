//
//  RenderTexture.cpp
//  Vortex
//

#include <Vortex/Renderer/RenderTexture.h>

#include <Vortex/Renderer/CommandBuffer.h>
#include <Vortex/Renderer/Drawable.h>

#include "Device.h"
#include "RenderPass.h"

namespace Vortex
{
namespace Renderer
{
namespace
{
auto MakeRenderPass(VulkanDevice& device, Format format)
{
  VkRenderPass renderPass =
      RenderpassBuilder()
          .Attachement(format)
          .AttachementLoadOp(vk::AttachmentLoadOp::eLoad)
          .AttachementStoreOp(vk::AttachmentStoreOp::eStore)
          // TODO should they both be general?
          .AttachementInitialLayout(vk::ImageLayout::eGeneral)
          .AttachementFinalLayout(vk::ImageLayout::eGeneral)
          .Subpass(vk::PipelineBindPoint::eGraphics)
          .SubpassColorAttachment(vk::ImageLayout::eColorAttachmentOptimal, 0)
          .Dependency(VK_SUBPASS_EXTERNAL, 0)
          .DependencySrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
          .DependencyDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
          .DependencySrcAccessMask(vk::AccessFlagBits::eColorAttachmentRead)
          .DependencyDstAccessMask(vk::AccessFlagBits::eColorAttachmentWrite)
          .Create(device.Handle());

  return reinterpret_cast<Handle::RenderPass>(renderPass);
}
}  // namespace

struct RenderTexture::Impl
{
  Impl(RenderTexture& self, Device& device)
      : mSelf(self), mDevice(static_cast<VulkanDevice&>(device))
  {
    // Create framebuffer
    vk::ImageView attachments[] = {Handle::ConvertImageView(mSelf.Texture::GetView())};

    auto framebufferInfo = vk::FramebufferCreateInfo()
                               .setWidth(mSelf.RenderTarget::GetWidth())
                               .setHeight(mSelf.RenderTarget::GetHeight())
                               .setRenderPass(reinterpret_cast<VkRenderPass>(mSelf.GetRenderPass()))
                               .setAttachmentCount(1)
                               .setPAttachments(attachments)
                               .setLayers(1);

    mFramebuffer = mDevice.Handle().createFramebufferUnique(framebufferInfo);
  }

  RenderCommand Record(DrawableList drawables, ColorBlendState blendState)
  {
    RenderState state(mSelf, blendState);
    return RenderCommand(
        mDevice, mSelf, state, Handle::ConvertFramebuffer(*mFramebuffer), drawables);
  }

  void Submit(RenderCommand& renderCommand) { renderCommand.Render(); }

  RenderTexture& mSelf;
  VulkanDevice& mDevice;
  vk::UniqueFramebuffer mFramebuffer;
};

RenderTexture::RenderTexture(Device& device, uint32_t width, uint32_t height, Format format)
    : RenderTarget(device,
                   width,
                   height,
                   MakeRenderPass(static_cast<VulkanDevice&>(device), format))
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
