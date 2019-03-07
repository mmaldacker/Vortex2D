//
//  RenderTarget.h
//  Vortex2D
//

#ifndef RenderTarget_h
#define RenderTarget_h

#include <Vortex2D/Renderer/Common.h>
#include <Vortex2D/Renderer/RenderState.h>

#include <functional>
#include <initializer_list>

namespace Vortex2D
{
namespace Renderer
{
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

  VORTEX2D_API virtual ~RenderTarget();

  // TODO should use shared_ptr?
  using DrawableList = std::initializer_list<std::reference_wrapper<Drawable>>;

  VORTEX2D_API virtual RenderCommand Record(DrawableList drawables, ColorBlendState blendState = {}) = 0;

  // TODO should use shared_ptr?
  VORTEX2D_API virtual void Submit(RenderCommand& renderCommand) = 0;

  uint32_t Width;
  uint32_t Height;
  glm::mat4 Orth;
  glm::mat4 View;
  vk::UniqueRenderPass RenderPass;
};

/**
 * @brief Factory for a vulkan render pass.
 */
class RenderpassBuilder
{
public:
  /**
   * @brief Format of the render pass
   * @param format
   * @return
   */
  RenderpassBuilder& Attachement(vk::Format format);

  /**
   * @brief operation to perform when loading the framebuffer (clear, load, etc)
   * @param value
   * @return
   */
  RenderpassBuilder& AttachementLoadOp(vk::AttachmentLoadOp value);

  /**
   * @brief operation to perform when storing the framebuffer (clear, save, etc)
   * @param value
   * @return
   */
  RenderpassBuilder& AttachementStoreOp(vk::AttachmentStoreOp value);

  /**
   * @brief Layout of the image to be before render pass
   * @param layout
   * @return
   */
  RenderpassBuilder& AttachementInitialLayout(vk::ImageLayout layout);

  /**
   * @brief Layout of the image to be after render pass
   * @param layout
   * @return
   */
  RenderpassBuilder& AttachementFinalLayout(vk::ImageLayout layout);

  /**
   * @brief Define subpass of the render pass
   * @param bindPoint
   * @return
   */
  RenderpassBuilder& Subpass(vk::PipelineBindPoint bindPoint);

  /**
   * @brief Set the color attachment with index
   * @param layout
   * @param attachment index of the attachment
   * @return
   */
  RenderpassBuilder& SubpassColorAttachment(vk::ImageLayout layout, uint32_t attachment);

  /**
   * @brief Dependency of the subpasses
   * @param srcSubpass
   * @param dstSubpass
   * @return
   */
  RenderpassBuilder& Dependency(uint32_t srcSubpass, uint32_t dstSubpass);

  RenderpassBuilder& DependencySrcStageMask(vk::PipelineStageFlags value);
  RenderpassBuilder& DependencyDstStageMask(vk::PipelineStageFlags value);
  RenderpassBuilder& DependencySrcAccessMask(vk::AccessFlags value);
  RenderpassBuilder& DependencyDstAccessMask(vk::AccessFlags value);

  /**
   * @brief Create the render pass
   * @param device
   * @return
   */
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

}  // namespace Renderer
}  // namespace Vortex2D

#endif
