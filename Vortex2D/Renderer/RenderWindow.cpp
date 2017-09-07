//
//  RenderWindow.cpp
//  Vortex2D
//

#include "RenderWindow.h"

namespace Vortex2D { namespace Renderer {

struct SwapChainSupportDetails
{
    SwapChainSupportDetails(vk::PhysicalDevice device, vk::SurfaceKHR surface)
        : capabilities(device.getSurfaceCapabilitiesKHR(surface))
        , formats(device.getSurfaceFormatsKHR(surface))
        , presentModes(device.getSurfacePresentModesKHR(surface))
    {
    }

    vk::SurfaceCapabilitiesKHR capabilities;
    std::vector<vk::SurfaceFormatKHR> formats;
    std::vector<vk::PresentModeKHR> presentModes;

    bool IsValid() const
    {
        return !formats.empty() && !presentModes.empty();
    }
};

RenderWindow::RenderWindow(const Device& device, vk::SurfaceKHR surface, uint32_t width, uint32_t height)
    : RenderTarget(width, height)
    , mDevice(device)
{
    // get swap chain support details
    SwapChainSupportDetails details(device.GetPhysicalDevice(), surface);
    if (!details.IsValid())
    {
        throw std::runtime_error("Swap chain support invalid");
    }

    // TODO verify this value is valid
    uint32_t numFramebuffers = details.capabilities.minImageCount + 1;

    // TODO choose given the details
    auto format = vk::Format::eB8G8R8A8Unorm;

    // TODO find if better mode is available
    auto mode = vk::PresentModeKHR::eFifo;

    // create swap chain
    auto swapChainInfo = vk::SwapchainCreateInfoKHR()
            .setSurface(surface)
            .setImageFormat(format)
            .setImageColorSpace(vk::ColorSpaceKHR::eSrgbNonlinear)
            .setMinImageCount(numFramebuffers)
            .setImageExtent({Width, Height})
            .setImageArrayLayers(1)
            .setImageUsage(vk::ImageUsageFlagBits::eColorAttachment)
            .setImageSharingMode(vk::SharingMode::eExclusive)
            .setPreTransform(details.capabilities.currentTransform)
            .setCompositeAlpha(vk::CompositeAlphaFlagBitsKHR::eOpaque)
            .setPresentMode(mode)
            .setClipped(true);

    mSwapChain = device.Handle().createSwapchainKHRUnique(swapChainInfo);

    // create swap chain image views
    std::vector<vk::Image> swapChainImages = device.Handle().getSwapchainImagesKHR(*mSwapChain);
    for (const auto& image : swapChainImages)
    {
        auto imageViewInfo = vk::ImageViewCreateInfo()
                .setImage(image)
                .setViewType(vk::ImageViewType::e2D)
                .setFormat(format)
                .setComponents({vk::ComponentSwizzle::eIdentity,
                                vk::ComponentSwizzle::eIdentity,
                                vk::ComponentSwizzle::eIdentity,
                                vk::ComponentSwizzle::eIdentity})
                .setSubresourceRange({vk::ImageAspectFlagBits::eColor, 0, 1, 0 ,1});

        mSwapChainImageViews.push_back(device.Handle().createImageViewUnique(imageViewInfo));
    }

    // Create render pass
    RenderPass = RenderpassBuilder()
            .Attachement(format)
            .AttachementLoadOp(vk::AttachmentLoadOp::eDontCare)
            .AttachementStoreOp(vk::AttachmentStoreOp::eStore)
            .AttachementFinalLayout(vk::ImageLayout::ePresentSrcKHR)
            .Subpass(vk::PipelineBindPoint::eGraphics)
            .SubpassColorAttachment(vk::ImageLayout::eColorAttachmentOptimal, 0)
            .Dependency(VK_SUBPASS_EXTERNAL, 0)
            .DependencySrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
            .DependencyDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
            .DependencyDstAccessMask(vk::AccessFlagBits::eColorAttachmentWrite)
            .Create(device.Handle());

    // Create framebuffers
    for (const auto& imageView : mSwapChainImageViews)
    {
        vk::ImageView attachments[] = {*imageView};
        auto framebufferInfo = vk::FramebufferCreateInfo()
                .setRenderPass(*RenderPass)
                .setAttachmentCount(1)
                .setPAttachments(attachments)
                .setWidth(Width)
                .setHeight(Height)
                .setLayers(1);

        mFrameBuffers.push_back(device.Handle().createFramebufferUnique(framebufferInfo));
    }

    // Create semaphores
    mImageAvailableSemaphore = device.Handle().createSemaphoreUnique({});
    mRenderFinishedSemaphore = device.Handle().createSemaphoreUnique({});

    // Create command Buffers
    mCmdBuffers = device.CreateCommandBuffers(numFramebuffers);
}

RenderWindow::~RenderWindow()
{
    mDevice.FreeCommandBuffers({mCmdBuffers});
}

void RenderWindow::Submit(std::initializer_list<vk::Semaphore> waitSemaphore,
                          std::initializer_list<vk::Semaphore> signalSemaphore)
{
    uint32_t imageIndex;
    auto result = mDevice.Handle().acquireNextImageKHR(*mSwapChain, UINT64_MAX, *mImageAvailableSemaphore, nullptr);
    if (result.result == vk::Result::eSuccess)
    {
        imageIndex = result.value;
    }
    else
    {
        throw std::runtime_error("Acquire error " + vk::to_string(result.result));
    }

    std::vector<vk::Semaphore> waitSemaphores = waitSemaphore;
    waitSemaphores.push_back(*mImageAvailableSemaphore);

    std::vector<vk::Semaphore> signalSemaphores = signalSemaphore;
    signalSemaphores.push_back(*mRenderFinishedSemaphore);

    std::vector<vk::PipelineStageFlags> waitStages(waitSemaphores.size(), vk::PipelineStageFlagBits::eAllCommands);

    auto submitInfo = vk::SubmitInfo()
            .setCommandBufferCount(1)
            .setPCommandBuffers(&mCmdBuffers[imageIndex])
            .setWaitSemaphoreCount(waitSemaphores.size())
            .setPWaitSemaphores(waitSemaphores.data())
            .setSignalSemaphoreCount(signalSemaphores.size())
            .setPSignalSemaphores(signalSemaphores.data())
            .setPWaitDstStageMask(waitStages.data());

    mDevice.Queue().submit({submitInfo}, nullptr);

    vk::SwapchainKHR swapChain[] = {*mSwapChain};

    vk::Semaphore presentSemaphore[] = {*mRenderFinishedSemaphore};

    auto presentInfo = vk::PresentInfoKHR()
            .setSwapchainCount(1)
            .setPSwapchains(swapChain)
            .setPImageIndices(&imageIndex)
            .setPWaitSemaphores(presentSemaphore)
            .setWaitSemaphoreCount(1);

    mDevice.Queue().presentKHR(presentInfo);
}

void RenderWindow::Record(CommandFn commandFn)
{
    for (uint32_t i = 0; i < mCmdBuffers.size(); i++)
    {
        auto bufferBegin = vk::CommandBufferBeginInfo()
                .setFlags(vk::CommandBufferUsageFlagBits::eSimultaneousUse);

        mCmdBuffers[i].begin(bufferBegin);

        auto renderPassBegin = vk::RenderPassBeginInfo()
                .setFramebuffer(*mFrameBuffers[i])
                .setRenderPass(*RenderPass)
                .setRenderArea({{0, 0}, {Width, Height}});

        mCmdBuffers[i].beginRenderPass(renderPassBegin, vk::SubpassContents::eInline);

        commandFn(mCmdBuffers[i]);

        mCmdBuffers[i].endRenderPass();
        mCmdBuffers[i].end();
    }
}

}}
