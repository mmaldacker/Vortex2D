//
//  RenderTarget.h
//  Vortex2D
//

#ifndef RenderTarget_h
#define RenderTarget_h

#include <Vortex2D/Renderer/Common.h>

#include <functional>
#include <initializer_list>

namespace Vortex2D { namespace Renderer {

class RenderState;
class RenderCommand;
struct Drawable;

/**
 * @brief A target that can be rendered to.
 * This is implemented by the @ref RenderWindow and the @ref RenderTexture
 */
struct RenderTarget
{
    RenderTarget(uint32_t width, uint32_t height);
    RenderTarget(RenderTarget&& other);

    virtual ~RenderTarget();

    // TODO should use shared_ptr?
    using DrawableList = std::initializer_list<std::reference_wrapper<Drawable>>;

    virtual RenderCommand Record(DrawableList drawables,
                                 vk::PipelineColorBlendAttachmentState blendMode = {}) = 0;

    // TODO should use shared_ptr?
    virtual void Submit(RenderCommand& renderCommand) = 0;

    uint32_t Width;
    uint32_t Height;
    glm::mat4 Orth;
    glm::mat4 View;
    vk::UniqueRenderPass RenderPass;
};

class RenderpassBuilder
{
public:
    RenderpassBuilder& Attachement(vk::Format format);
    RenderpassBuilder& AttachementLoadOp(vk::AttachmentLoadOp value);
    RenderpassBuilder& AttachementStoreOp(vk::AttachmentStoreOp value);
    RenderpassBuilder& AttachementInitialLayout(vk::ImageLayout layout);
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
