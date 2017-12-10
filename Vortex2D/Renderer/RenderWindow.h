//
//  RenderWindow.h
//  Vortex2D
//

#ifndef RenderWindow_h
#define RenderWindow_h

#include <Vortex2D/Renderer/Common.h>
#include <Vortex2D/Renderer/RenderTarget.h>
#include <Vortex2D/Renderer/Device.h>
#include <Vortex2D/Renderer/Pipeline.h>

namespace Vortex2D { namespace Renderer {

class RenderWindow : public RenderTarget
{
public:
    RenderWindow(const Device& device, vk::SurfaceKHR surface, uint32_t width, uint32_t height);

    RenderCommand Record(DrawableList drawables,
                         vk::PipelineColorBlendAttachmentState blendMode = {}) override;
    void Submit(RenderCommand& renderCommand) override;

    void Display();

private:
    const Device& mDevice;
    vk::UniqueSwapchainKHR mSwapChain;
    std::vector<vk::UniqueImageView> mSwapChainImageViews;
    std::vector<vk::UniqueFramebuffer> mFrameBuffers;
    vk::UniqueSemaphore mImageAvailableSemaphore;
    vk::UniqueSemaphore mRenderFinishedSemaphore;
    std::vector<std::reference_wrapper<RenderCommand>> mRenderCommands;
    uint32_t mIndex;
};

}}

#endif
