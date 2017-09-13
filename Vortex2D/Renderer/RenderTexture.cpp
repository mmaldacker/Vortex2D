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
            .Create(mDevice.Handle());

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

    // Create Command Buffer
    mCmd = mDevice.CreateCommandBuffers(1).at(0);
}

RenderTexture::~RenderTexture()
{
    mDevice.FreeCommandBuffers({mCmd});
}

void RenderTexture::Record(CommandFn commandFn)
{
    auto bufferBegin = vk::CommandBufferBeginInfo()
            .setFlags(vk::CommandBufferUsageFlagBits::eSimultaneousUse);

    mCmd.begin(bufferBegin);

    auto renderPassBegin = vk::RenderPassBeginInfo()
            .setFramebuffer(*mFramebuffer)
            .setRenderPass(*RenderPass)
            .setRenderArea({{0, 0}, {Width, Height}});

    mCmd.beginRenderPass(renderPassBegin, vk::SubpassContents::eInline);

    commandFn(mCmd);

    mCmd.endRenderPass();
    mCmd.end();
}

void RenderTexture::Submit(std::initializer_list<vk::Semaphore> waitSemaphore,
                           std::initializer_list<vk::Semaphore> signalSemaphore)
{
    std::vector<vk::PipelineStageFlags> waitStages(waitSemaphore.size(), vk::PipelineStageFlagBits::eAllCommands);

    auto submitInfo = vk::SubmitInfo()
            .setCommandBufferCount(1)
            .setPCommandBuffers(&mCmd)
            .setWaitSemaphoreCount(waitSemaphore.size())
            .setPWaitSemaphores(waitSemaphore.begin())
            .setSignalSemaphoreCount(signalSemaphore.size())
            .setPSignalSemaphores(signalSemaphore.begin())
            .setPWaitDstStageMask(waitStages.data());

    mDevice.Queue().submit({submitInfo}, nullptr);
}

bool RenderTexture::operator==(const RenderTexture& other) const
{
    return *mFramebuffer == *other.mFramebuffer;
}

}}
