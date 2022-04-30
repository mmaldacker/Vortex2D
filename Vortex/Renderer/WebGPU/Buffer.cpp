//
//  Buffer.cpp
//  Vortex2D
//

#include <Vortex/Renderer/Buffer.h>

#include <Vortex/Renderer/CommandBuffer.h>
#include <Vortex/Renderer/Texture.h>

#include "Device.h"

namespace Vortex
{
namespace Renderer
{
void BufferMapCallback(WGPUBufferMapAsyncStatus status, void* userdata)
{
  auto promise = reinterpret_cast<std::promise<void>*>(userdata);
  if (status == WGPUBufferMapAsyncStatus_Success)
  {
    promise->set_value();
  }
  else
  {
    promise->set_exception(std::make_exception_ptr(std::runtime_error("buffer read error")));
  }
}

WGPUBufferUsageFlags GetBufferUsage(BufferUsage usage, MemoryUsage memoryUsage)
{
  if (memoryUsage == MemoryUsage::Gpu)
  {
    return ConvertBufferUsage(usage) | ConvertMemoryUsage(memoryUsage);
  }
  else
  {
    return ConvertMemoryUsage(memoryUsage);
  }
}

struct GenericBuffer::Impl
{
  WebGPUDevice& mDevice;
  WGPUBufferUsageFlags mUsageFlags;
  std::uint64_t mSize;
  WGPUBuffer mBuffer;

  Impl(Device& device, BufferUsage usageFlags, MemoryUsage memoryUsage, std::uint64_t deviceSize)
      : mDevice(static_cast<WebGPUDevice&>(device))
      , mUsageFlags(GetBufferUsage(usageFlags, memoryUsage))
      , mSize(deviceSize)
      , mBuffer(0)
  {
    Create();
  }

  ~Impl()
  {
    if (mBuffer != 0)
    {
      // FIXME buffer destroy
      // wgpuBufferDestroy(mBuffer);
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

    mBuffer = wgpuDeviceCreateBuffer(mDevice.Handle(), &descriptor);
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
      // FIXME buffer destroy
      // wgpuBufferDestroy(mBuffer);
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

    wgpuCommandEncoderCopyBufferToBuffer(Handle::ConvertCommandEncoder(command.Handle()),
                                         Handle::ConvertBuffer(srcBuffer.Handle()),
                                         0,
                                         mBuffer,
                                         0,
                                         mSize);
  }

  void CopyFrom(CommandEncoder& command, Texture& srcTexture)
  {
    auto textureSize =
        GetBytesPerPixel(srcTexture.GetFormat()) * srcTexture.GetWidth() * srcTexture.GetHeight();
    if (textureSize > mSize)
    {
      throw std::runtime_error("Cannot copy texture of different sizes");
    }

    WGPUImageCopyTexture src{};
    src.texture = Handle::ConvertImage(srcTexture.Handle());

    WGPUTextureDataLayout layout{};
    layout.bytesPerRow =
        GetPaddedBytes(GetBytesPerPixel(srcTexture.GetFormat()) * srcTexture.GetWidth(), 256);

    WGPUImageCopyBuffer dst{};
    dst.buffer = mBuffer;
    dst.layout = layout;

    WGPUExtent3D extent{};
    extent.width = srcTexture.GetWidth();
    extent.height = srcTexture.GetHeight();
    extent.depth = 1;

    wgpuCommandEncoderCopyTextureToBuffer(
        Handle::ConvertCommandEncoder(command.Handle()), &src, &dst, &extent);
  }

  void Barrier(CommandEncoder& /*command*/, Access /*oldAccess*/, Access /*newAccess*/) {}

  void Clear(CommandEncoder& command)
  {
    // TODO implement
  }

  void CopyFrom(uint32_t offset, const void* data, uint32_t size)
  {
    std::promise<void> promise;
    wgpuBufferMapAsync(
        mBuffer, WGPUMapMode_Write, offset, size, BufferMapCallback, (uint8_t*)&promise);

    // TODO why is this needed? maybe queue needs to be empty?
    wgpuDevicePoll(mDevice.Handle(), false);

    promise.get_future().get();

    auto* dstData = (uint8_t*)wgpuBufferGetMappedRange(mBuffer, offset, size);

    std::memcpy(dstData, data, size);

    wgpuBufferUnmap(mBuffer);
  }

  void CopyTo(uint32_t offset, void* data, uint32_t size)
  {
    std::promise<void> promise;
    wgpuBufferMapAsync(
        mBuffer, WGPUMapMode_Read, offset, size, BufferMapCallback, (uint8_t*)&promise);

    // TODO why is this needed?  maybe queue needs to be empty?
    wgpuDevicePoll(mDevice.Handle(), false);

    promise.get_future().get();

    const auto* srcData = (const uint8_t*)wgpuBufferGetMappedRange(mBuffer, offset, size);
    if (srcData == nullptr)
    {
      throw std::runtime_error("Could not map buffer");
    }

    std::memcpy(data, srcData, size);

    wgpuBufferUnmap(mBuffer);
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
}  // namespace Vortex
