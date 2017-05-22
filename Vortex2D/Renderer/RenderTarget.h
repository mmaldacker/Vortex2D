//
//  RenderTarget.h
//  Vortex2D
//

#ifndef RenderTarget_h
#define RenderTarget_h

#include <Vortex2D/Renderer/Common.h>

#include <functional>

namespace Vortex2D { namespace Renderer {

class GraphicsPipeline;

/**
 * @brief And interface to represent a target that can be rendered to.
 * This is implemented by the RenderWindow (in the examples) and the RenderTexture
 */
struct RenderTarget
{
    RenderTarget(float width, float height);

    using CommandFn = std::function<void(vk::CommandBuffer, vk::RenderPass)>;
    virtual ~RenderTarget() {}
    virtual void Create(GraphicsPipeline&) = 0;
    virtual void Record(CommandFn) = 0;
    virtual void Submit() = 0;

    glm::mat4 Orth;
};

class RenderpassBuilder
{
public:
    RenderpassBuilder& Attachement(vk::Format format);
    RenderpassBuilder& AttachementLoadOp(vk::AttachmentLoadOp value);
    RenderpassBuilder& AttachementStoreOp(vk::AttachmentStoreOp value);
    RenderpassBuilder& AttachementFinalLayout(vk::ImageLayout layout);

    RenderpassBuilder& Subpass(vk::PipelineBindPoint bindPoint);
    RenderpassBuilder& SubpassColorAttachment(vk::ImageLayout layout, uint32_t attachment);

    RenderpassBuilder& Dependency(uint32_t srcSubpass, uint32_t dstSubpass);
    RenderpassBuilder& DependencySrcStageMask(vk::PipelineStageFlags value);
    RenderpassBuilder& DependencyDstStageMask(vk::PipelineStageFlags value);
    RenderpassBuilder& DependencySrcAccessMask(vk::AccessFlags value);
    RenderpassBuilder& DependencyDstAccessMask(vk::AccessFlags value);

    vk::UniqueRenderPass Create(vk::Device device);

private:
    constexpr static int mMaxRefs = 64;

    vk::AttachmentReference* GetAttachmentReference();

    std::vector<vk::AttachmentDescription> mAttachementDescriptions;
    std::vector<vk::SubpassDescription> mSubpassDescriptions;
    std::vector<vk::SubpassDependency> mSubpassDependencies;

    int mNumRefs = 0;
    std::array<vk::AttachmentReference, mMaxRefs> mAttachmentReferences;

};

}}

#endif
