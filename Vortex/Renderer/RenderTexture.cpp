//
//  RenderTexture.cpp
//  Vortex
//

#include "RenderTexture.h"

#include <Vortex/Renderer/CommandBuffer.h>
#include <Vortex/Renderer/Drawable.h>

namespace Vortex
{
namespace Renderer
{
RenderTexture::RenderTexture(Device& device,
                             uint32_t width,
                             uint32_t height,
                             vk::Format format)
    : RenderTarget(width, height), Texture(device, width, height, format), mDevice(device)
{
  // Create render pass
  RenderPass = RenderpassBuilder()
                   .Attachement(format)
                   .AttachementLoadOp(vk::AttachmentLoadOp::eLoad)
                   .AttachementStoreOp(vk::AttachmentStoreOp::eStore)
                   .AttachementInitialLayout(vk::ImageLayout::eGeneral)
                   .AttachementFinalLayout(vk::ImageLayout::eGeneral)
                   .Subpass(vk::PipelineBindPoint::eGraphics)
                   .SubpassColorAttachment(vk::ImageLayout::eColorAttachmentOptimal, 0)
                   .Dependency(VK_SUBPASS_EXTERNAL, 0)
                   .DependencySrcStageMask(vk::PipelineStageFlagBits::eBottomOfPipe)
                   .DependencyDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
                   .DependencySrcAccessMask(vk::AccessFlagBits::eMemoryRead)
                   .DependencyDstAccessMask(vk::AccessFlagBits::eColorAttachmentWrite |
                                            vk::AccessFlagBits::eColorAttachmentRead)
                   .DependencyFlag(vk::DependencyFlagBits::eByRegion)
                   .Dependency(0, VK_SUBPASS_EXTERNAL)
                   .DependencySrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
                   .DependencyDstStageMask(vk::PipelineStageFlagBits::eBottomOfPipe)
                   .DependencySrcAccessMask(vk::AccessFlagBits::eColorAttachmentWrite |
                                            vk::AccessFlagBits::eColorAttachmentRead)
                   .DependencyDstAccessMask(vk::AccessFlagBits::eMemoryRead)
                   .DependencyFlag(vk::DependencyFlagBits::eByRegion)
                   .Create(device.Handle());

  // Create framebuffer
  vk::ImageView attachments[] = {GetView()};

  auto framebufferInfo = vk::FramebufferCreateInfo()
                             .setWidth(Width)
                             .setHeight(Height)
                             .setRenderPass(*RenderPass)
                             .setAttachmentCount(1)
                             .setPAttachments(attachments)
                             .setLayers(1);

  mFramebuffer = device.Handle().createFramebufferUnique(framebufferInfo);
}

RenderTexture::RenderTexture(RenderTexture&& other)
    : RenderTarget(std::move(other))
    , Texture(std::move(other))
    , mDevice(other.mDevice)
    , mFramebuffer(std::move(other.mFramebuffer))
{
}

RenderTexture::~RenderTexture() {}

RenderCommand RenderTexture::Record(DrawableList drawables, ColorBlendState blendState)
{
  RenderState state(*this, blendState);
  return RenderCommand(mDevice, *this, state, mFramebuffer, drawables);
}

void RenderTexture::Submit(RenderCommand& renderCommand)
{
  renderCommand.Render();
}

}  // namespace Renderer
}  // namespace Vortex
