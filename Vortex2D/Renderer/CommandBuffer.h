//
//  CommandBuffer.h
//  Vortex
//

#pragma once

#include <Vortex2D/Renderer/Common.h>
#include <Vortex2D/Renderer/RenderTarget.h>

#include <functional>
#include <initializer_list>
#include <vector>

namespace Vortex
{
namespace Renderer
{
class Device;

/**
 * @brief Can record commands, then submit them (multiple times).
 * A fence can used to wait on the completion of the commands.
 */
class CommandBuffer
{
public:
  using CommandFn = std::function<void(vk::CommandBuffer)>;

  /**
   * @brief Creates a command buffer which can be synchronized.
   * @param device vulkan device
   * @param synchronise flag to determine if the command buffer can be waited
   * on.
   */
  VORTEX_API explicit CommandBuffer(const Device& device, bool synchronise = true);
  VORTEX_API ~CommandBuffer();

  VORTEX_API CommandBuffer(CommandBuffer&&);
  VORTEX_API CommandBuffer& operator=(CommandBuffer&&);

  /**
   * @brief Record some commands. The commads are recorded in the lambda which
   * is immediately executed.
   * @param commandFn a functor, or simply a lambda, where commands are
   * recorded.
   */
  VORTEX_API CommandBuffer& Record(CommandFn commandFn);

  /**
   * @brief Record some commands inside a render pass. The commads are recorded
   * in the lambda which is immediately executed.
   * @param renderTarget the render target which contains the render pass to
   * record into
   * @param framebuffer the frame buffer where the render pass will render.
   * @param commandFn a functor, or simply a lambda, where commands are
   * recorded.
   */
  VORTEX_API CommandBuffer& Record(const RenderTarget& renderTarget,
                                   vk::Framebuffer framebuffer,
                                   CommandFn commandFn);

  /**
   * @brief Wait for the command submit to finish. Does nothing if the
   * synchronise flag was false.
   */
  VORTEX_API CommandBuffer& Wait();

  /**
   * @brief Reset the command buffer so it can be recorded again.
   */
  VORTEX_API CommandBuffer& Reset();

  /**
   * @brief submit the command buffer
   */
  VORTEX_API CommandBuffer& Submit(
      const std::initializer_list<vk::Semaphore>& waitSemaphores = {},
      const std::initializer_list<vk::Semaphore>& signalSemaphores = {});

  /**
   * @brief explicit conversion operator to bool, indicates if the command was
   * properly recorded and can be sumitted.
   */
  VORTEX_API explicit operator bool() const;

private:
  const Device& mDevice;
  bool mSynchronise;
  bool mRecorded;
  vk::CommandBuffer mCommandBuffer;
  vk::UniqueFence mFence;
};

/**
 * @brief A special command buffer that has been recorded by a @ref
 * RenderTarget. It can be used to submit the rendering. The object has to stay
 * alive untill rendering is complete.
 */
class RenderCommand
{
public:
  VORTEX_API RenderCommand();
  VORTEX_API ~RenderCommand();

  VORTEX_API RenderCommand(RenderCommand&&);
  VORTEX_API RenderCommand& operator=(RenderCommand&&);

  /**
   * @brief Submit the render command with a transform matrix
   * @param view a transform matrix
   * @return *this
   */
  VORTEX_API RenderCommand& Submit(const glm::mat4& view = glm::mat4(1.0f));

  /**
   * @brief Wait for the render command to complete
   */
  VORTEX_API void Wait();

  /**
   * @brief explicit conversion operator to bool, indicates if the command was
   * properly recorded and can be sumitted.
   */
  VORTEX_API explicit operator bool() const;

  friend class RenderTexture;
  friend class RenderWindow;

private:
  RenderCommand(const Device& device,
                RenderTarget& renderTarget,
                const RenderState& renderState,
                const vk::UniqueFramebuffer& frameBuffer,
                RenderTarget::DrawableList drawables);

  RenderCommand(const Device& device,
                RenderTarget& renderTarget,
                const RenderState& renderState,
                const std::vector<vk::UniqueFramebuffer>& frameBuffers,
                const uint32_t& index,
                RenderTarget::DrawableList drawables);

  void Render(const std::initializer_list<vk::Semaphore>& waitSemaphores = {},
              const std::initializer_list<vk::Semaphore>& signalSemaphores = {});

  RenderTarget* mRenderTarget;
  std::vector<CommandBuffer> mCmds;
  const uint32_t* mIndex;
  std::vector<std::reference_wrapper<Drawable>> mDrawables;
  glm::mat4 mView;
};

}  // namespace Renderer
}  // namespace Vortex
