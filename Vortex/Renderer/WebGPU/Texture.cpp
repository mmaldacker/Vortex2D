//
//  Texture.cpp
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
struct Sampler::Impl
{
  Impl(Device& device, AddressMode addressMode, Filter filter) {}

  Handle::Sampler Handle() { return {}; }
};

Sampler::Sampler(Device& device, AddressMode addressMode, Filter filter)
    : mImpl(std::make_unique<Impl>(device, addressMode, filter))
{
}

Sampler::Sampler(Sampler&& other) : mImpl(std::move(other.mImpl)) {}

Sampler::~Sampler() {}

Handle::Sampler Sampler::Handle()
{
  return mImpl->Handle();
}

struct Texture::Impl
{
  Texture& mSelf;
  WebGPUDevice& mDevice;
  uint32_t mWidth;
  uint32_t mHeight;
  Format mFormat;
  WGPUTexture mId;
  WGPUTextureView mViewId;
  MemoryUsage mMemoryUsage;
  std::unique_ptr<GenericBuffer> mStagingBuffer;
  std::unique_ptr<CommandBuffer> mStagingCopyFromCmd, mStagingCopyToCmd;

  Impl(Texture& self,
       Device& device,
       uint32_t width,
       uint32_t height,
       Format format,
       MemoryUsage memoryUsage)
      : mSelf(self)
      , mDevice(static_cast<WebGPUDevice&>(device))
      , mWidth(width)
      , mHeight(height)
      , mFormat(format)
      , mId(0)
      , mViewId(0)
      , mMemoryUsage(memoryUsage)
  {
    WGPUTextureDescriptor textureDesc{};
    textureDesc.sampleCount = 1;
    textureDesc.mipLevelCount = 1;
    textureDesc.size = {mWidth, mHeight, 1};
    textureDesc.dimension = WGPUTextureDimension_2D;
    textureDesc.format = ConvertTextureFormat(mFormat);
    textureDesc.usage = ConvertTextureUsage(memoryUsage);

    mId = wgpuDeviceCreateTexture(mDevice.Handle(), &textureDesc);
    if (mId == 0)
    {
      throw std::runtime_error("Error creating texture");
    }

    if (memoryUsage == MemoryUsage::Gpu)
    {
      WGPUTextureViewDescriptor textureViewDesc{};
      textureViewDesc.format = ConvertTextureFormat(mFormat);
      textureViewDesc.dimension = WGPUTextureViewDimension_2D;
      textureViewDesc.aspect = WGPUTextureAspect_All;
      textureViewDesc.arrayLayerCount = 1;
      textureViewDesc.mipLevelCount = 1;

      mViewId = wgpuTextureCreateView(mId, &textureViewDesc);
      if (mViewId == 0)
      {
        throw std::runtime_error("Error creating texture view");
      }
    }
    else
    {
      std::int64_t size = GetPaddedBytes(GetBytesPerPixel(mFormat) * mWidth, 256) * mHeight;
      mStagingBuffer =
          std::make_unique<GenericBuffer>(mDevice, BufferUsage::Storage, memoryUsage, size);
    }
  }

  void Init()
  {
    if (mStagingBuffer)
    {
      if (mMemoryUsage == MemoryUsage::CpuToGpu)
      {
        mStagingCopyFromCmd = std::make_unique<CommandBuffer>(mDevice);
        mStagingCopyFromCmd->Record(
            [&](CommandEncoder& command) { CopyFrom(command, *mStagingBuffer); });
      }

      if (mMemoryUsage == MemoryUsage::GpuToCpu)
      {
        mStagingCopyToCmd = std::make_unique<CommandBuffer>(mDevice);
        mStagingCopyToCmd->Record(
            [&](CommandEncoder& command) { mStagingBuffer->CopyFrom(command, mSelf); });
      }
    }
  }

  ~Impl()
  {
    if (mViewId != 0)
    {
      // TODO do we need to destroy mViewId?
    }

    if (mId != 0)
    {
      // FIXME texture destroy
      // wgpuTextureDestroy(mId);
    }
  }

  Impl(Impl&& other)
      : mSelf(other.mSelf)
      , mDevice(other.mDevice)
      , mWidth(other.mWidth)
      , mHeight(other.mHeight)
      , mFormat(other.mFormat)
      , mId(other.mId)
      , mViewId(other.mViewId)
  {
    other.mId = 0;
    other.mViewId = 0;
  }

  void Clear(CommandEncoder& command, const std::array<int, 4>& colour) {}

  void Clear(CommandEncoder& command, const std::array<float, 4>& colour) {}

  void CopyFrom(const void* data, int size)
  {
    // TODO very inefficient

    std::int64_t rowSize = GetBytesPerPixel(mFormat) * mWidth;
    std::int64_t paddedRowSize = GetPaddedBytes(rowSize, 256);

    std::vector<uint8_t> alignedBuffer(mStagingBuffer->Size());
    for (int i = 0; i < mHeight; i++)
    {
      std::memcpy(
          alignedBuffer.data() + i * paddedRowSize, (const uint8_t*)data + i * rowSize, rowSize);
    }

    mStagingBuffer->CopyFrom(0, alignedBuffer.data(), alignedBuffer.size());

    mStagingCopyFromCmd->Submit();
    mStagingCopyFromCmd->Wait();
  }

  void CopyTo(void* data, int size)
  {
    // TODO very inefficient

    mStagingCopyToCmd->Submit();
    mStagingCopyToCmd->Wait();

    std::vector<uint8_t> alignedBuffer(mStagingBuffer->Size());
    mStagingBuffer->CopyTo(0, alignedBuffer.data(), alignedBuffer.size());

    std::int64_t rowSize = GetBytesPerPixel(mFormat) * mWidth;
    std::int64_t paddedRowSize = GetPaddedBytes(rowSize, 256);

    for (int i = 0; i < mHeight; i++)
    {
      std::memcpy((uint8_t*)data + i * rowSize, alignedBuffer.data() + i * paddedRowSize, rowSize);
    }
  }

  void CopyFrom(CommandEncoder& command, Texture& srcImage)
  {
    if (mWidth != srcImage.GetWidth() && mHeight != srcImage.GetHeight() &&
        mFormat != srcImage.GetFormat())
    {
      throw std::runtime_error("Copying texture of different size/format");
    }

    WGPUImageCopyTexture srcView{};
    srcView.texture = Handle::ConvertTexture(srcImage.Handle());

    WGPUImageCopyTexture dstView{};
    dstView.texture = mId;

    WGPUExtent3D extent{};
    extent.width = mWidth;
    extent.height = mHeight;
    extent.depth = 1;

    wgpuCommandEncoderCopyTextureToTexture(
        Handle::ConvertCommandEncoder(command.Handle()), &srcView, &dstView, &extent);
  }

  void CopyFrom(CommandEncoder& command, GenericBuffer& srcBuffer)
  {
    WGPUTextureDataLayout layout{};
    layout.bytesPerRow = GetPaddedBytes(GetBytesPerPixel(mFormat) * mWidth, 256);

    WGPUImageCopyBuffer src{};
    src.buffer = Handle::ConvertBuffer(srcBuffer.Handle());
    src.layout = layout;

    WGPUImageCopyTexture dst{};
    dst.texture = Handle::ConvertTexture(Handle());

    WGPUExtent3D extent{};
    extent.width = mWidth;
    extent.height = mHeight;
    extent.depth = 1;

    wgpuCommandEncoderCopyBufferToTexture(
        Handle::ConvertCommandEncoder(command.Handle()), &src, &dst, &extent);
  }

  void Barrier(CommandEncoder& command,
               ImageLayout oldLayout,
               Access srcMask,
               ImageLayout newLayout,
               Access dstMask)
  {
  }

  Handle::ImageView GetView() const { return reinterpret_cast<Handle::ImageView>(mViewId); }

  uint32_t GetWidth() const { return mWidth; }

  uint32_t GetHeight() const { return mHeight; }

  Format GetFormat() const { return mFormat; }

  Handle::Image Handle() const { return reinterpret_cast<Handle::Image>(mId); }
};

Texture::Texture(Device& device,
                 uint32_t width,
                 uint32_t height,
                 Format format,
                 MemoryUsage memoryUsage)
    : mImpl(std::make_unique<Impl>(*this, device, width, height, format, memoryUsage))
{
  mImpl->Init();
}

Texture::Texture(Texture&& other) : mImpl(std::move(other.mImpl)) {}

Texture::~Texture() {}

void Texture::CopyFrom(const void* data, int size)
{
  mImpl->CopyFrom(data, size);
}

void Texture::CopyTo(void* data, int size)
{
  mImpl->CopyTo(data, size);
}

void Texture::CopyFrom(CommandEncoder& command, Texture& srcImage)
{
  mImpl->CopyFrom(command, srcImage);
}

void Texture::CopyFrom(CommandEncoder& command, GenericBuffer& srcBuffer)
{
  mImpl->CopyFrom(command, srcBuffer);
}

void Texture::Barrier(CommandEncoder& command,
                      ImageLayout oldLayout,
                      Access oldAccess,
                      ImageLayout newLayout,
                      Access newAccess)
{
  mImpl->Barrier(command, oldLayout, oldAccess, newLayout, newAccess);
}

Handle::ImageView Texture::GetView() const
{
  return mImpl->GetView();
}

uint32_t Texture::GetWidth() const
{
  return mImpl->GetWidth();
}

uint32_t Texture::GetHeight() const
{
  return mImpl->GetHeight();
}

Format Texture::GetFormat() const
{
  return mImpl->GetFormat();
}

void Texture::Clear(CommandEncoder& command, const std::array<int, 4>& colour)
{
  mImpl->Clear(command, colour);
}

void Texture::Clear(CommandEncoder& command, const std::array<float, 4>& colour)
{
  mImpl->Clear(command, colour);
}

Handle::Image Texture::Handle() const
{
  return mImpl->Handle();
}

}  // namespace Renderer
}  // namespace Vortex
