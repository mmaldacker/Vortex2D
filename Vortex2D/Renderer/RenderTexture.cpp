//
//  RenderTexture.cpp
//  Vortex
//

#include "RenderTexture.h"

#include <Vortex2D/Renderer/Drawable.h>

namespace Vortex2D { namespace Renderer {

RenderTexture::RenderTexture(const Device& device, uint32_t width, uint32_t height, vk::Format format)
    : RenderTarget(width, height)
    , Texture(device, width, height, format, false)
    , mCmd(device)
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
    vk::ImageView attachments[] = {*this};

    auto framebufferInfo = vk::FramebufferCreateInfo()
            .setWidth(Width)
            .setHeight(Height)
            .setRenderPass(*RenderPass)
            .setAttachmentCount(1)
            .setPAttachments(attachments)
            .setLayers(1);

    mFramebuffer = device.Handle().createFramebufferUnique(framebufferInfo);
}

void RenderTexture::Record(DrawableList drawables,
                           vk::PipelineColorBlendAttachmentState blendMode)
{
    RenderState state(*this, blendMode);

    for (auto& drawable: drawables)
    {
      drawable.get().Initialize(state);
    }

    mCmd.Record(*this, *mFramebuffer, [&](vk::CommandBuffer commandBuffer)
    {
        for (auto& drawable: drawables)
        {
          drawable.get().Draw(commandBuffer, state);
        }
    });
}

void RenderTexture::Submit(std::initializer_list<vk::Semaphore> waitSemaphore,
                           std::initializer_list<vk::Semaphore> signalSemaphore)
{
    // TODO add (or remove?) wait semaphore
    mCmd.Submit(signalSemaphore);
}

bool RenderTexture::operator==(const RenderTexture& other) const
{
    return *mFramebuffer == *other.mFramebuffer;
}

}}
