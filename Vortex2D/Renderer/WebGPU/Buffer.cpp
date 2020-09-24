//
//  Buffer.cpp
//  Vortex2D
//

#include <Vortex2D/Renderer/Buffer.h>

#include <Vortex2D/Renderer/CommandBuffer.h>
#include <Vortex2D/Renderer/Texture.h>

#include "Device.h"

namespace Vortex2D
{
namespace Renderer
{
void BufferMapReadCallback(WGPUBufferMapAsyncStatus status, const uint8_t* data, uint8_t* userdata)
{
  auto promise = reinterpret_cast<std::promise<const uint8_t*>*>(userdata);
  if (status == WGPUBufferMapAsyncStatus_Success)
  {
    promise->set_value(data);
  }
  else
  {
    promise->set_exception(std::make_exception_ptr(std::runtime_error("buffer read error")));
  }
}

void BufferMapWriteCallback(WGPUBufferMapAsyncStatus status, uint8_t* data, uint8_t* userdata)
{
  auto promise = reinterpret_cast<std::promise<uint8_t*>*>(userdata);
  if (status == WGPUBufferMapAsyncStatus_Success)
  {
    promise->set_value(data);
  }
  else
  {
    promise->set_exception(std::make_exception_ptr(std::runtime_error("buffer write error")));
  }
}

struct GenericBuffer::Impl
{
  WebGPUDevice& mDevice;
  WGPUBufferUsage mUsageFlags;
  std::uint64_t mSize;
  WGPUBufferId mBuffer;

  Impl(Device& device, BufferUsage usageFlags, MemoryUsage memoryUsage, std::uint64_t deviceSize)
      : mDevice(static_cast<WebGPUDevice&>(device))
      , mUsageFlags(ConvertBufferUsage(usageFlags) | ConvertMemoryUsage(memoryUsage))
      , mSize(deviceSize)
      , mBuffer(0)
  {
    Create();
  }

  ~Impl()
  {
    if (mBuffer != 0)
    {
      wgpu_buffer_destroy(mBuffer);
      mBuffer = 0;
    }
  }

  Impl(Impl&& other) : mDevice(other.mDevice), mUsageFlags(other.mUsageFlags), mSize(other.mSize)
  {
    other.mSize = 0;
    other.mBuffer = 0;
  }

  void Create()
  {
    WGPUBufferDescriptor descriptor{};
    descriptor.size = mSize;
    descriptor.usage = mUsageFlags;

    mBuffer = wgpu_device_create_buffer(mDevice.Handle(), &descriptor);
    if (mBuffer == 0)
    {
      throw std::runtime_error("Error creating buffer");
    }
  }

  Handle::Buffer Handle() const { return reinterpret_cast<Handle::Buffer>(mBuffer); }

  std::uint64_t Size() const { return mSize; }

  void Resize(std::uint64_t size)
  {
    if (mBuffer != 0)
    {
      wgpu_buffer_destroy(mBuffer);
    }

    mSize = size;
    Create();
  }

  void CopyFrom(CommandEncoder& command, GenericBuffer& srcBuffer)
  {
    if (mSize != srcBuffer.Size())
    {
      throw std::runtime_error("Cannot copy buffers of different sizes");
    }

    wgpu_command_encoder_copy_buffer_to_buffer(Handle::ConvertCommandEncoder(command.Handle()),
                                               Handle::ConvertBuffer(srcBuffer.Handle()),
                                               0,
                                               mBuffer,
                                               0,
                                               mSize);
  }

  void CopyFrom(CommandEncoder& command, Texture& srcTexture)
  {
    auto textureSize =
        srcTexture.GetWidth() * srcTexture.GetHeight() * GetBytesPerPixel(srcTexture.GetFormat());
    if (textureSize != mSize)
    {
      throw std::runtime_error("Cannot copy texture of different sizes");
    }

    WGPUTextureCopyView src{};
    src.texture = Handle::ConvertImage(srcTexture.Handle());

    WGPUTextureDataLayout layout{};
    layout.offset = 0;
    layout.bytes_per_row = GetBytesPerPixel(srcTexture.GetFormat()) * srcTexture.GetWidth();
    layout.rows_per_image = srcTexture.GetHeight();

    WGPUBufferCopyView dst{};
    dst.buffer = mBuffer;
    dst.layout = layout;

    WGPUExtent3d extent{};
    extent.width = srcTexture.GetWidth();
    extent.height = srcTexture.GetHeight();
    extent.depth = 1;

    wgpu_command_encoder_copy_texture_to_buffer(
        Handle::ConvertCommandEncoder(command.Handle()), &src, &dst, &extent);
  }

  void Barrier(CommandEncoder& /*command*/, Access /*oldAccess*/, Access /*newAccess*/) {}

  void Clear(CommandEncoder& command)
  {
    // TODO implement
  }

  void CopyFrom(uint32_t offset, const void* data, uint32_t size)
  {
    std::promise<uint8_t*> promise;
    wgpu_buffer_map_write_async(mBuffer, offset, size, BufferMapWriteCallback, (uint8_t*)&promise);

    wgpu_device_poll(mDevice.Handle(), false);

    uint8_t* dstData = promise.get_future().get();
    std::memcpy(dstData, data, size);

    wgpu_buffer_unmap(mBuffer);
  }

  void CopyTo(uint32_t offset, void* data, uint32_t size)
  {
    std::promise<const uint8_t*> promise;
    wgpu_buffer_map_read_async(mBuffer, offset, size, BufferMapReadCallback, (uint8_t*)&promise);

    wgpu_device_poll(mDevice.Handle(), false);

    const uint8_t* srcData = promise.get_future().get();
    std::memcpy(data, srcData, size);

    wgpu_buffer_unmap(mBuffer);
  }
};

GenericBuffer::GenericBuffer(Device& device,
                             BufferUsage usageFlags,
                             MemoryUsage memoryUsage,
                             std::uint64_t deviceSize)
    : mImpl(std::make_unique<GenericBuffer::Impl>(device, usageFlags, memoryUsage, deviceSize))
{
}

GenericBuffer::GenericBuffer(GenericBuffer&& other) : mImpl(std::move(other.mImpl)) {}

GenericBuffer::~GenericBuffer() {}

void GenericBuffer::CopyFrom(CommandEncoder& command, GenericBuffer& srcBuffer)
{
  mImpl->CopyFrom(command, srcBuffer);
}

void GenericBuffer::CopyFrom(CommandEncoder& command, Texture& srcTexture)
{
  mImpl->CopyFrom(command, srcTexture);
}

Handle::Buffer GenericBuffer::Handle() const
{
  return mImpl->Handle();
}

std::uint64_t GenericBuffer::Size() const
{
  return mImpl->Size();
}

void GenericBuffer::Resize(std::uint64_t size)
{
  mImpl->Resize(size);
}

void GenericBuffer::Barrier(CommandEncoder& command, Access oldAccess, Access newAccess)
{
  mImpl->Barrier(command, oldAccess, newAccess);
}

void GenericBuffer::Clear(CommandEncoder& command)
{
  mImpl->Clear(command);
}

void GenericBuffer::CopyFrom(uint32_t offset, const void* data, uint32_t size)
{
  mImpl->CopyFrom(offset, data, size);
}

void GenericBuffer::CopyTo(uint32_t offset, void* data, uint32_t size)
{
  mImpl->CopyTo(offset, data, size);
}

}  // namespace Renderer
}  // namespace Vortex2D
