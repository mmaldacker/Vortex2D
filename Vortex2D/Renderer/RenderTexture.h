//
//  RenderTexture.h
//  Vortex2D
//

#ifndef RenderTexture_h
#define RenderTexture_h

#include <Vortex2D/Renderer/Common.h>
#include <Vortex2D/Renderer/Device.h>
#include <Vortex2D/Renderer/Texture.h>
#include <Vortex2D/Renderer/RenderTarget.h>
#include <Vortex2D/Renderer/Pipeline.h>
#include <Vortex2D/Renderer/CommandBuffer.h>

namespace Vortex2D { namespace Renderer {

class RenderTexture : public RenderTarget, public Texture
{
public:
    RenderTexture(const Device& device, uint32_t width, uint32_t height, vk::Format format);

    void Record(DrawableList drawables,
                vk::PipelineColorBlendAttachmentState blendMode = {}) override;
    void Submit(std::initializer_list<vk::Semaphore> waitSemaphore = {},
                std::initializer_list<vk::Semaphore> signalSemaphore = {}) override;

    bool operator==(const RenderTexture& other) const;

private:
    vk::UniqueFramebuffer mFramebuffer;
    RenderCommandBuffer mCmd;
};

}}

#endif
