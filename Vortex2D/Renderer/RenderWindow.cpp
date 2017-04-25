//
//  RenderWindow.cpp
//  Vortex2D
//

#include "RenderWindow.h"

#include <stdexcept>

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
    , mWidth(width)
    , mHeight(height)
{
    // get swap chain support details
    SwapChainSupportDetails details(device.GetPhysicalDevice(), surface);
    if (!details.IsValid())
    {
        throw std::runtime_error("Swap chain support invalid");
    }

    // create swap chain
    vk::SwapchainCreateInfoKHR swapChainInfo;
    swapChainInfo
            .setSurface(surface)
            // TODO choose given the details
            .setImageFormat(vk::Format::eB8G8R8A8Unorm)
            .setImageColorSpace(vk::ColorSpaceKHR::eSrgbNonlinear)
            .setMinImageCount(details.capabilities.minImageCount + 1)
            .setImageExtent({mWidth, mHeight})
            .setImageArrayLayers(1)
            .setImageUsage(vk::ImageUsageFlagBits::eColorAttachment)
            .setImageSharingMode(vk::SharingMode::eExclusive)
            .setPreTransform(details.capabilities.currentTransform)
            .setCompositeAlpha(vk::CompositeAlphaFlagBitsKHR::eOpaque)
            // TODO find if better mode is available
            .setPresentMode(vk::PresentModeKHR::eFifo)
            .setClipped(true);

    mSwapChain = device.GetDevice().createSwapchainKHRUnique(swapChainInfo);

    // create swap chain image views
    std::vector<vk::Image> swapChainImages = device.GetDevice().getSwapchainImagesKHR(*mSwapChain);
    for (const auto& image : swapChainImages)
    {
        vk::ImageViewCreateInfo imageViewInfo;
        imageViewInfo
                .setImage(image)
                .setViewType(vk::ImageViewType::e2D)
                // TODO set same as for swapChainInfo
                .setFormat(vk::Format::eB8G8R8A8Unorm)
                .setComponents({vk::ComponentSwizzle::eIdentity, vk::ComponentSwizzle::eIdentity, vk::ComponentSwizzle::eIdentity, vk::ComponentSwizzle::eIdentity})
                .setSubresourceRange({vk::ImageAspectFlagBits::eColor, 0, 1, 0 ,1});

       mSwapChainImageViews.push_back(device.GetDevice().createImageViewUnique(imageViewInfo));
    }

    // Create render pass
    vk::AttachmentDescription colorAttachment;
    colorAttachment
            .setLoadOp(vk::AttachmentLoadOp::eClear)
            .setStoreOp(vk::AttachmentStoreOp::eStore)
            .setInitialLayout(vk::ImageLayout::eUndefined)
            .setFinalLayout(vk::ImageLayout::ePresentSrcKHR)
            .setFormat(vk::Format::eB8G8R8A8Unorm);

    vk::AttachmentReference colorAttachmentRef;
    colorAttachmentRef
            .setAttachment(0)
            .setLayout(vk::ImageLayout::eColorAttachmentOptimal);

    vk::SubpassDescription subpass;
    subpass
            .setColorAttachmentCount(1)
            .setPColorAttachments(&colorAttachmentRef);

    vk::RenderPassCreateInfo renderPassInfo;
    renderPassInfo
            .setAttachmentCount(1)
            .setPAttachments(&colorAttachment)
            .setSubpassCount(1)
            .setPSubpasses(&subpass);

    mRenderPass = device.GetDevice().createRenderPassUnique(renderPassInfo);

    // Create framebuffers
    for (const auto& imageView : mSwapChainImageViews)
    {
        vk::ImageView attachments[] = {*imageView};
        vk::FramebufferCreateInfo framebufferInfo;
        framebufferInfo
                .setRenderPass(*mRenderPass)
                .setAttachmentCount(1)
                .setPAttachments(attachments)
                .setWidth(mWidth)
                .setHeight(mHeight)
                .setLayers(1);

        mFrameBuffers.push_back(device.GetDevice().createFramebufferUnique(framebufferInfo));
    }
}

RenderWindow::~RenderWindow()
{
}

void RenderWindow::Clear(const glm::vec4 & colour)
{
}

void RenderWindow::Render(Vortex2D::Renderer::Drawable & object, const glm::mat4 & transform)
{
    //object.Render(*this, transform);
}

}}
