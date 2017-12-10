//
//  RenderTexture.cpp
//  Vortex
//

#include "RenderTexture.h"

#include <Vortex2D/Renderer/Drawable.h>
#include <Vortex2D/Renderer/CommandBuffer.h>

namespace Vortex2D { namespace Renderer {

RenderTexture::RenderTexture(const Device& device, uint32_t width, uint32_t height, vk::Format format)
    : RenderTarget(width, height)
    , Texture(device, width, height, format, false)
    , mDevice(device)
{
    // Create render pass
    RenderPass = RenderpassBuilder()
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

RenderCommand RenderTexture::Record(DrawableList drawables,
                                    vk::PipelineColorBlendAttachmentState blendMode)
{
    RenderState state(*this, blendMode);
    return RenderCommand(mDevice, *this, state, mFramebuffer, drawables);
}

void RenderTexture::Submit(RenderCommand& renderCommand)
{
    renderCommand.Render();
}

bool RenderTexture::operator==(const RenderTexture& other) const
{
    return *mFramebuffer == *other.mFramebuffer;
}

}}
