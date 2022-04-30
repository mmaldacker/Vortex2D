//
//  RenderWindow.cpp
//  Vortex
//

#include <Vortex/Renderer/RenderWindow.h>

#include <Vortex/Renderer/Vulkan/RenderPass.h>

#include <Vortex/Renderer/CommandBuffer.h>
#include <Vortex/Renderer/Drawable.h>

#include "Device.h"

namespace Vortex
{
namespace Renderer
{
void TextureBarrier(Handle::Image image,
                    Handle::CommandBuffer commandBuffer,
                    vk::ImageLayout oldLayout,
                    vk::AccessFlags srcMask,
                    vk::ImageLayout newLayout,
                    vk::AccessFlags dstMask);

namespace
{
Handle::RenderPass MakeRenderPass(VulkanDevice& device, Format format)
{
  // Create render pass
  VkRenderPass renderPass =
      RenderpassBuilder()
          .Attachement(format)
          .AttachementLoadOp(vk::AttachmentLoadOp::eLoad)
          .AttachementStoreOp(vk::AttachmentStoreOp::eStore)
          .AttachementInitialLayout(vk::ImageLayout::ePresentSrcKHR)
          .AttachementFinalLayout(vk::ImageLayout::ePresentSrcKHR)
          .Subpass(vk::PipelineBindPoint::eGraphics)
          .SubpassColorAttachment(vk::ImageLayout::eColorAttachmentOptimal, 0)
          .Dependency(VK_SUBPASS_EXTERNAL, 0)
          .DependencySrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
          .DependencyDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
          .DependencyDstAccessMask(vk::AccessFlagBits::eColorAttachmentWrite)
          .Create(device.Handle());

  return reinterpret_cast<Handle::RenderPass>(renderPass);
}
}  // namespace

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

  bool IsValid() const { return !formats.empty() && !presentModes.empty(); }
};

struct RenderWindow::Impl
{
  Impl(RenderWindow& self, Device& device, Handle::Surface surface)
      : mSelf(self), mDevice(static_cast<VulkanDevice&>(device)), mIndex(0), mFrameIndex(0)
  {
    // get swap chain support details
    SwapChainSupportDetails details(mDevice.GetPhysicalDevice(), Handle::ConvertSurface(surface));
    if (!details.IsValid())
    {
      throw std::runtime_error("Swap chain support invalid");
    }

    // TODO verify this value is valid
    uint32_t numFramebuffers = details.capabilities.minImageCount + 1;

    // TODO choose given the details
    auto format = Format::B8G8R8A8Unorm;

    // TODO find if better mode is available
    auto mode = vk::PresentModeKHR::eFifo;

    // create swap chain
    auto swapChainInfo = vk::SwapchainCreateInfoKHR()
                             .setSurface(Handle::ConvertSurface(surface))
                             .setImageFormat(ConvertFormat(format))
                             .setImageColorSpace(vk::ColorSpaceKHR::eSrgbNonlinear)
                             .setMinImageCount(numFramebuffers)
                             .setImageExtent({mSelf.GetWidth(), mSelf.GetHeight()})
                             .setImageArrayLayers(1)
                             .setImageUsage(vk::ImageUsageFlagBits::eColorAttachment |
                                            vk::ImageUsageFlagBits::eTransferDst)
                             .setImageSharingMode(vk::SharingMode::eExclusive)
                             .setPreTransform(details.capabilities.currentTransform)
                             .setCompositeAlpha(vk::CompositeAlphaFlagBitsKHR::eOpaque)
                             .setPresentMode(mode)
                             .setClipped(true);

    mSwapChain = mDevice.Handle().createSwapchainKHRUnique(swapChainInfo);

    // create swap chain image views
    std::vector<vk::Image> swapChainImages = mDevice.Handle().getSwapchainImagesKHR(*mSwapChain);
    for (const auto& image : swapChainImages)
    {
      auto imageViewInfo = vk::ImageViewCreateInfo()
                               .setImage(image)
                               .setViewType(vk::ImageViewType::e2D)
                               .setFormat(ConvertFormat(format))
                               .setComponents({vk::ComponentSwizzle::eIdentity,
                                               vk::ComponentSwizzle::eIdentity,
                                               vk::ComponentSwizzle::eIdentity,
                                               vk::ComponentSwizzle::eIdentity})
                               .setSubresourceRange({vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1});

      mSwapChainImageViews.push_back(mDevice.Handle().createImageViewUnique(imageViewInfo));

      mDevice.Execute(
          [&](CommandEncoder& command)
          {
            TextureBarrier(Handle::ConvertImage(image),
                           command.Handle(),
                           vk::ImageLayout::eUndefined,
                           vk::AccessFlags{},
                           vk::ImageLayout::eGeneral,
                           vk::AccessFlagBits::eTransferWrite);

            auto clearValue = vk::ClearColorValue().setFloat32({{0.0f, 0.0f, 0.0f, 0.0f}});

            vk::CommandBuffer commandBuffer = reinterpret_cast<VkCommandBuffer>(command.Handle());
            commandBuffer.clearColorImage(
                image,
                vk::ImageLayout::eGeneral,
                clearValue,
                vk::ImageSubresourceRange{vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1});

            TextureBarrier(Handle::ConvertImage(image),
                           command.Handle(),
                           vk::ImageLayout::eGeneral,
                           vk::AccessFlagBits::eTransferWrite,
                           vk::ImageLayout::ePresentSrcKHR,
                           vk::AccessFlags{});
          });
    }

    // Create framebuffers
    for (const auto& imageView : mSwapChainImageViews)
    {
      vk::ImageView attachments[] = {*imageView};
      auto framebufferInfo =
          vk::FramebufferCreateInfo()
              .setRenderPass(reinterpret_cast<VkRenderPass>(mSelf.GetRenderPass()))
              .setAttachmentCount(1)
              .setPAttachments(attachments)
              .setWidth(mSelf.GetWidth())
              .setHeight(mSelf.GetHeight())
              .setLayers(1);

      mFrameBuffers.push_back(mDevice.Handle().createFramebufferUnique(framebufferInfo));

      // Create semaphores
      mImageAvailableSemaphores.push_back(mDevice.Handle().createSemaphoreUnique({}));
      mRenderFinishedSemaphores.push_back(mDevice.Handle().createSemaphoreUnique({}));
    }
  }

  ~Impl() {}

  RenderCommand Record(RenderTarget::DrawableList drawables, ColorBlendState blendState)
  {
    RenderState state(mSelf, blendState);

    std::vector<Handle::Framebuffer> framebuffers;
    for (auto& frameBuffer : mFrameBuffers)
    {
      framebuffers.push_back(Handle::ConvertFramebuffer(*frameBuffer));
    }

    return RenderCommand(mDevice, mSelf, state, framebuffers, mIndex, drawables);
  }

  void Submit(RenderCommand& renderCommand) { mRenderCommands.emplace_back(renderCommand); }

  void Display()
  {
    if (mRenderCommands.empty())
      return;  // nothing to draw

    auto result = mDevice.Handle().acquireNextImageKHR(
        *mSwapChain, UINT64_MAX, *mImageAvailableSemaphores[mFrameIndex], nullptr);
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
      mRenderCommands[0].get().Render(
          {Handle::ConvertSemaphore(*mImageAvailableSemaphores[mFrameIndex])},
          {Handle::ConvertSemaphore(*mRenderFinishedSemaphores[mFrameIndex])});
    }
    else
    {
      mRenderCommands.front().get().Render(
          {Handle::ConvertSemaphore(*mImageAvailableSemaphores[mFrameIndex])});
      for (std::size_t i = 1; i < mRenderCommands.size() - 1; i++)
      {
        mRenderCommands[i].get().Render();
      }
      mRenderCommands.back().get().Render(
          {}, {Handle::ConvertSemaphore(*mRenderFinishedSemaphores[mFrameIndex])});
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

  RenderWindow& mSelf;
  VulkanDevice& mDevice;
  vk::UniqueSwapchainKHR mSwapChain;
  std::vector<vk::UniqueImageView> mSwapChainImageViews;
  std::vector<vk::UniqueFramebuffer> mFrameBuffers;
  std::vector<vk::UniqueSemaphore> mImageAvailableSemaphores;
  std::vector<vk::UniqueSemaphore> mRenderFinishedSemaphores;
  std::vector<std::reference_wrapper<RenderCommand>> mRenderCommands;
  uint32_t mIndex;
  uint32_t mFrameIndex;
};

RenderWindow::RenderWindow(Device& device, Handle::Surface surface, uint32_t width, uint32_t height)
    : RenderTarget(device,
                   width,
                   height,
                   MakeRenderPass(static_cast<VulkanDevice&>(device),
                                  Format::B8G8R8A8Unorm))  // TODO choose format
    , mImpl(std::make_unique<Impl>(*this, device, surface))
{
}

RenderWindow::~RenderWindow() {}

RenderCommand RenderWindow::Record(DrawableList drawables, ColorBlendState blendState)
{
  return mImpl->Record(drawables, blendState);
}

void RenderWindow::Submit(RenderCommand& renderCommand)
{
  mImpl->Submit(renderCommand);
}

void RenderWindow::Display()
{
  mImpl->Display();
}

}  // namespace Renderer
}  // namespace Vortex
