//
//  RenderWindow.cpp
//  Vortex2D
//

#include "RenderWindow.h"

#include <stdexcept>

namespace Vortex2D { namespace Renderer {

RenderWindow::RenderWindow(vk::Device device, int width, int height)
    : Vortex2D::Renderer::RenderTarget(width, height)
    , mWidth(width)
    , mHeight(height)
{
    vk::AttachmentDescription colorAttachment;
    colorAttachment
            .setLoadOp(vk::AttachmentLoadOp::eClear)
            .setStoreOp(vk::AttachmentStoreOp::eStore)
            .setInitialLayout(vk::ImageLayout::eUndefined)
            .setFinalLayout(vk::ImageLayout::ePresentSrcKHR);

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

    vk::UniqueRenderPass renderPass = device.createRenderPassUnique(renderPassInfo);
}

RenderWindow::~RenderWindow()
{
}

void RenderWindow::Clear(const glm::vec4 & colour)
{
}

void RenderWindow::Render(Vortex2D::Renderer::Drawable & object, const glm::mat4 & transform)
{


    object.Render(*this, transform);
}

}}
