//
//  Device.h
//  Vortex2D
//

#ifndef Vortex2d_Device_h
#define Vortex2d_Device_h

#include <Vortex2D/Renderer/CommandBuffer.h>
#include <Vortex2D/Renderer/Common.h>
#include <Vortex2D/Renderer/DescriptorSet.h>
#include <Vortex2D/Utils/vk_mem_alloc.h>
#include <map>

namespace Vortex2D
{
namespace Renderer
{
/**
 * @brief A binary SPIRV shader, to be feed to vulkan.
 */
class SpirvBinary
{
public:
  template <std::size_t N>
  SpirvBinary(const uint32_t (&spirv)[N]) : mData(spirv), mSize(N * 4)
  {
  }

  const uint32_t* data() const { return mData; }

  std::size_t size() const { return mSize; }

  std::size_t words() const { return mSize / 4; }

private:
  const uint32_t* mData;
  std::size_t mSize;
};

/**
 * @brief Encapsulation around the vulkan device. Allows to create command
 * buffers, layout, bindings, memory and shaders.
 */
class Device
{
public:
  VORTEX2D_API Device(vk::PhysicalDevice physicalDevice, bool validation = true);
  VORTEX2D_API Device(vk::PhysicalDevice physicalDevice,
                      vk::SurfaceKHR surface,
                      bool validation = true);
  VORTEX2D_API Device(vk::PhysicalDevice physicalDevice, int familyIndex, bool validation = true);
  VORTEX2D_API ~Device();

  Device(Device&&) = delete;
  Device& operator=(Device&&) = delete;

  // Vulkan handles and helpers
  VORTEX2D_API vk::Device Handle() const;
  VORTEX2D_API vk::Queue Queue() const;
  VORTEX2D_API vk::PhysicalDevice GetPhysicalDevice() const;
  VORTEX2D_API int GetFamilyIndex() const;

  // Command buffer functions
  VORTEX2D_API vk::CommandBuffer CreateCommandBuffer() const;
  VORTEX2D_API void FreeCommandBuffer(vk::CommandBuffer commandBuffer) const;
  VORTEX2D_API void Execute(CommandBuffer::CommandFn commandFn) const;

  // Memory allocator
  VORTEX2D_API VmaAllocator Allocator() const;
  VORTEX2D_API LayoutManager& GetLayoutManager() const;

  VORTEX2D_API vk::ShaderModule GetShaderModule(const SpirvBinary& spirv) const;

private:
  vk::PhysicalDevice mPhysicalDevice;
  int mFamilyIndex;
  vk::UniqueDevice mDevice;
  vk::Queue mQueue;
  vk::UniqueCommandPool mCommandPool;
  vk::UniqueDescriptorPool mDescriptorPool;
  VmaAllocator mAllocator;

  mutable std::unique_ptr<CommandBuffer> mCommandBuffer;
  mutable std::map<const uint32_t*, vk::UniqueShaderModule> mShaders;
  mutable LayoutManager mLayoutManager;
};

}  // namespace Renderer
}  // namespace Vortex2D

#endif
