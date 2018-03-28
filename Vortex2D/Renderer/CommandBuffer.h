//
//  CommandBuffer.h
//  Vortex2D
//

#ifndef Vortex2d_CommandBuffer_h
#define Vortex2d_CommandBuffer_h

#include <Vortex2D/Renderer/Common.h>
#include <Vortex2D/Renderer/Device.h>
#include <Vortex2D/Renderer/RenderTarget.h>

#include <vector>
#include <functional>
#include <initializer_list>

namespace Vortex2D { namespace Renderer {

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
     * @param synchronise flag to determine if the command buffer can be waited on.
     */
    CommandBuffer(const Device& device, bool synchronise = true);
    ~CommandBuffer();

    CommandBuffer(CommandBuffer&&);
    CommandBuffer& operator=(CommandBuffer&&);

    /**
     * @brief Record some commands. The commads are recorded in the lambda which is immediately executed.
     * @param commandFn a functor, or simply a lambda, where commands are recorded.
     */
    void Record(CommandFn commandFn);

    /**
     * @brief Record some commands inside a render pass. The commads are recorded in the lambda which is immediately executed.
     * @param renderTarget the render target which contains the render pass to record into
     * @param framebuffer the frame buffer where the render pass will render.
     * @param commandFn a functor, or simply a lambda, where commands are recorded.
     */
    void Record(const RenderTarget& renderTarget, vk::Framebuffer framebuffer, CommandFn commandFn);

    /**
     * @brief Wait for the command submit to finish. Does nothing if the synchronise flag was false.
     */
    void Wait();

    /**
     * @brief Reset the command buffer so it can be recorded again.
     */
    void Reset();

    /**
      * @brief submit the command buffer
      */
    void Submit(const std::initializer_list<vk::Semaphore>& waitSemaphores = {},
                const std::initializer_list<vk::Semaphore>& signalSemaphores = {});

    /**
     * @brief explicit conversion operator to bool, indicates if the command was properly recorded and can be sumitted.
     */
    explicit operator bool() const;

private:
    const Device& mDevice;
    bool mSynchronise;
    bool mRecorded;
    vk::CommandBuffer mCommandBuffer;
    vk::UniqueFence mFence;
};

void ExecuteCommand(const Device& device, CommandBuffer::CommandFn commandFn);

/**
 * @brief A special command buffer that has been recorded by a @ref RenderTarget.
 * It can be used to submit the rendering. The object has to stay alive untill rendering is complete.
 */
class RenderCommand
{
public:
    RenderCommand();
    ~RenderCommand();

    RenderCommand(RenderCommand&&);
    RenderCommand& operator=(RenderCommand&&);

    /**
     * @brief Submit the render command with a transform matrix
     * @param view a transform matrix
     */
    void Submit(const glm::mat4& view = glm::mat4());

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



}}

#endif
