//
//  RenderWindow.h
//  Vortex2D
//

#ifndef RenderWindow_h
#define RenderWindow_h

#include <Vortex2D/Renderer/RenderTarget.h>
#include <Vortex2D/Renderer/Device.h>
#include <string>

namespace Vortex2D { namespace Renderer {


class RenderWindow : public RenderTarget
{
public:
    RenderWindow(const Device& device, vk::SurfaceKHR surface, uint32_t width, uint32_t height);

    virtual ~RenderWindow();

    void Clear(const glm::vec4 & colour);
    void Render(Drawable & object, const glm::mat4 & transform);

private:
    uint32_t mWidth, mHeight;
    vk::UniqueSwapchainKHR mSwapChain;
    vk::UniqueRenderPass mRenderPass;
    std::vector<vk::UniqueImageView> mSwapChainImageViews;
    std::vector<vk::UniqueFramebuffer> mFrameBuffers;
};

}}

#endif
