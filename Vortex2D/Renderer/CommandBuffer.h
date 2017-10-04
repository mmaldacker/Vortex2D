//
//  CommandBuffer.h
//  Vortex2D
//

#ifndef CommandBuffer_h
#define CommandBuffer_h

#include <Vortex2D/Renderer/Common.h>
#include <Vortex2D/Renderer/Device.h>

#include <vector>
#include <functional>
#include <initializer_list>

namespace Vortex2D { namespace Renderer {

struct RenderTarget;

class CommandBuffer
{
public:
    using CommandFn = std::function<void(vk::CommandBuffer)>;

    CommandBuffer(const Device& device, bool synchronise = true);
    ~CommandBuffer();

    CommandBuffer(CommandBuffer&&);
    CommandBuffer& operator=(CommandBuffer&&);

    void Record(CommandFn commandFn);
    void Wait();
    void Submit(std::initializer_list<vk::Semaphore> signalSemaphore = {});

private:
    const Device& mDevice;
    bool mSynchronise;
    vk::CommandBuffer mCommandBuffer;
    vk::UniqueFence mFence;
};

class RenderCommandBuffer
{
public:
    using CommandFn = std::function<void(vk::CommandBuffer)>;

    RenderCommandBuffer(const Device& device);
    ~RenderCommandBuffer();

    RenderCommandBuffer(RenderCommandBuffer&& other);
    RenderCommandBuffer& operator=(RenderCommandBuffer&& other);

    void Record(const RenderTarget& renderTarget, vk::Framebuffer framebuffer, CommandFn commandFn);
    void Submit(std::initializer_list<vk::Semaphore> signalSemaphore = {});

private:
    const Device& mDevice;
    vk::CommandBuffer mCommandBuffer;
};

void ExecuteCommand(const Device& device, CommandBuffer::CommandFn commandFn);

}}

#endif
