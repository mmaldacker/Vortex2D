//
//  RenderWindow.cpp
//  Vortex2D
//

#include "RenderWindow.h"

#include <Vortex2D/Renderer/Drawable.h>
#include <Vortex2D/Renderer/CommandBuffer.h>

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
    , mIndex(0)
    , mFrameIndex(0)
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
            .AttachementLoadOp(vk::AttachmentLoadOp::eLoad)
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
        
        // Create semaphores
        mImageAvailableSemaphores.push_back(device.Handle().createSemaphoreUnique({}));
        mRenderFinishedSemaphores.push_back(device.Handle().createSemaphoreUnique({}));
    }
}

RenderCommand RenderWindow::Record(DrawableList drawables,
                                   vk::PipelineColorBlendAttachmentState blendMode)
{
    RenderState state(*this, blendMode);
    return RenderCommand(mDevice,
                         *this,
                         state,
                         mFrameBuffers,
                         mIndex,
                         drawables);
}

void RenderWindow::Submit(RenderCommand& renderCommand)
{
    mRenderCommands.emplace_back(renderCommand);
}

void RenderWindow::Display()
{
    if (mRenderCommands.empty()) return; // nothing to draw

    auto result = mDevice.Handle().acquireNextImageKHR(*mSwapChain, UINT64_MAX, *mImageAvailableSemaphores[mFrameIndex], nullptr);
    if (result.result == vk::Result::eSuccess)
    {
        mIndex = result.value;
    }
    else
    {
        throw std::runtime_error("Acquire error " + vk::to_string(result.result));
    }

    if (mRenderCommands.size() == 1)
    {
        mRenderCommands[0].get().Render({*mImageAvailableSemaphores[mFrameIndex]}, {*mRenderFinishedSemaphores[mFrameIndex]});
    }
    else
    {
        mRenderCommands.front().get().Render({*mImageAvailableSemaphores[mFrameIndex]});
        for (std::size_t i = 1; i < mRenderCommands.size() - 1; i++)
        {
            mRenderCommands[i].get().Render();
        }
        mRenderCommands.back().get().Render({}, {*mRenderFinishedSemaphores[mFrameIndex]});
    }

    vk::SwapchainKHR swapChain[] = {*mSwapChain};
    vk::Semaphore waitSemaphores[] = {*mRenderFinishedSemaphores[mFrameIndex]};

    auto presentInfo = vk::PresentInfoKHR()
            .setSwapchainCount(1)
            .setPSwapchains(swapChain)
            .setPImageIndices(&mIndex)
            .setPWaitSemaphores(waitSemaphores)
            .setWaitSemaphoreCount(1);

    mDevice.Queue().presentKHR(presentInfo);
    mRenderCommands.clear();

    mFrameIndex = (mFrameIndex + 1) % mFrameBuffers.size();
}

}}
