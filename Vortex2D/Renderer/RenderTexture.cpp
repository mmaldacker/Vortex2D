//
//  RenderTexture.cpp
//  Vortex
//

#include "RenderTexture.h"

namespace Vortex2D { namespace Renderer {

RenderTexture::RenderTexture(const Device& device, uint32_t width, uint32_t height, vk::Format format)
    : RenderTarget(width, height)
    , Texture(device, width, height, format, false)
    , mDevice(device)
{
    // Create render pass
    mRenderPass = RenderpassBuilder()
            .Attachement(format)
            .AttachementLoadOp(vk::AttachmentLoadOp::eDontCare)
            .AttachementStoreOp(vk::AttachmentStoreOp::eStore)
            .AttachementFinalLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
            .Subpass(vk::PipelineBindPoint::eGraphics)
            .SubpassColorAttachment(vk::ImageLayout::eColorAttachmentOptimal, 0)
            .Dependency(VK_SUBPASS_EXTERNAL, 0)
            // TODO still not 100% sure what values below should be
            .DependencySrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
            .DependencyDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
            .DependencySrcAccessMask(vk::AccessFlagBits::eMemoryRead)
            .DependencyDstAccessMask(vk::AccessFlagBits::eColorAttachmentRead |
                                     vk::AccessFlagBits::eColorAttachmentWrite)
            .Create(mDevice.Handle());

    // Create framebuffer
    vk::ImageView attachments[] = {*this};

    auto framebufferInfo = vk::FramebufferCreateInfo()
            .setWidth(Width())
            .setHeight(Height())
            .setRenderPass(*mRenderPass)
            .setAttachmentCount(1)
            .setPAttachments(attachments)
            .setLayers(1);

    mFramebuffer = device.Handle().createFramebufferUnique(framebufferInfo);

    // Create Command Buffer
    mCmd = mDevice.CreateCommandBuffers(1).at(0);

    // Create fence
    auto fenceInfo = vk::FenceCreateInfo()
            .setFlags(vk::FenceCreateFlagBits::eSignaled);

    mFence = mDevice.Handle().createFenceUnique(fenceInfo);
}

RenderTexture::~RenderTexture()
{
    mDevice.FreeCommandBuffers({mCmd});
}

void RenderTexture::Create(GraphicsPipeline& pipeline)
{
    pipeline.Create(mDevice.Handle(), Width(), Height(), *mRenderPass);
}

void RenderTexture::Record(CommandFn commandFn)
{
    auto bufferBegin = vk::CommandBufferBeginInfo()
            .setFlags(vk::CommandBufferUsageFlagBits::eSimultaneousUse);

    mCmd.begin(bufferBegin);

    auto renderPassBegin = vk::RenderPassBeginInfo()
            .setFramebuffer(*mFramebuffer)
            .setRenderPass(*mRenderPass)
            .setRenderArea({{0, 0}, {Width(), Height()}});

    mCmd.beginRenderPass(renderPassBegin, vk::SubpassContents::eInline);

    commandFn(mCmd, *mRenderPass);

    mCmd.endRenderPass();
    mCmd.end();
}

void RenderTexture::Submit()
{
    mDevice.Handle().waitForFences({*mFence}, true, UINT64_MAX);
    mDevice.Handle().resetFences({*mFence});

    auto submitInfo = vk::SubmitInfo()
            .setCommandBufferCount(1)
            .setPCommandBuffers(&mCmd);

    mDevice.Queue().submit({submitInfo}, *mFence);
}

}}
