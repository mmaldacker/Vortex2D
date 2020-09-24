//
//  Texture.cpp
//  Vortex2D
//

#include <Vortex2D/Renderer/CommandBuffer.h>
#include <Vortex2D/Renderer/Texture.h>

#include "Device.h"

namespace Vortex2D
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
  WGPUTextureId mId;
  WGPUTextureViewId mViewId;

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
  {
    WGPUTextureDescriptor textureDesc{};
    textureDesc.sample_count = 1;
    textureDesc.mip_level_count = 1;
    textureDesc.size = {mWidth, mHeight, 1};
    textureDesc.dimension = WGPUTextureDimension_D2;
    textureDesc.format = ConvertTextureFormat(mFormat);
    textureDesc.usage = ConvertTextureUsage(memoryUsage);

    mId = wgpu_device_create_texture(mDevice.Handle(), &textureDesc);
    if (mId == 0)
    {
      throw std::runtime_error("Error creating texture");
    }

    if (memoryUsage != MemoryUsage::Cpu)
    {
      WGPUTextureViewDescriptor textureViewDesc{};
      textureViewDesc.format = ConvertTextureFormat(mFormat);
      textureViewDesc.dimension = WGPUTextureViewDimension_D2;
      textureViewDesc.aspect = WGPUTextureAspect_All;
      textureViewDesc.array_layer_count = 1;
      textureViewDesc.level_count = 1;

      mViewId = wgpu_texture_create_view(mId, &textureViewDesc);
      if (mViewId == 0)
      {
        throw std::runtime_error("Error creating texture view");
      }
    }
  }

  ~Impl()
  {
    if (mId != 0)
    {
      wgpu_texture_destroy(mId);
    }

    if (mViewId != 0)
    {
      wgpu_texture_view_destroy(mViewId);
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

  void CopyFrom(const void* data)
  {
    // TODO very ineficient

    std::int64_t size = GetBytesPerPixel(mFormat) * mWidth * mHeight;
    GenericBuffer stagingBuffer(mDevice, BufferUsage::Storage, MemoryUsage::Cpu, size);

    stagingBuffer.CopyFrom(0, data, size);

    mDevice.Execute([&](CommandEncoder& command) { CopyFrom(command, stagingBuffer); });
  }

  void CopyTo(void* data)
  {
    // TODO very ineficient

    std::int64_t size = GetBytesPerPixel(mFormat) * mWidth * mHeight;
    GenericBuffer stagingBuffer(mDevice, BufferUsage::Storage, MemoryUsage::Cpu, size);

    mDevice.Execute([&](CommandEncoder& command) { stagingBuffer.CopyFrom(command, mSelf); });

    stagingBuffer.CopyTo(0, data, size);
  }

  void CopyFrom(CommandEncoder& command, Texture& srcImage)
  {
    if (mWidth != srcImage.GetWidth() && mHeight != srcImage.GetHeight() &&
        mFormat != srcImage.GetFormat())
    {
      throw std::runtime_error("Copying texture of different size/format");
    }

    WGPUTextureCopyView srcView{};
    srcView.texture = Handle::ConvertTexture(srcImage.Handle());

    WGPUTextureCopyView dstView{};
    dstView.texture = Handle::ConvertTexture(Handle());

    WGPUExtent3d extent{};
    extent.width = mWidth;
    extent.height = mHeight;
    extent.depth = 1;

    wgpu_command_encoder_copy_texture_to_texture(
        Handle::ConvertCommandEncoder(command.Handle()), &srcView, &dstView, &extent);
  }

  void CopyFrom(CommandEncoder& command, GenericBuffer& srcBuffer)
  {
    WGPUTextureDataLayout layout{};
    layout.offset = 0;
    layout.bytes_per_row = GetBytesPerPixel(mFormat) * mWidth;
    layout.rows_per_image = mHeight;

    WGPUBufferCopyView src{};
    src.buffer = Handle::ConvertBuffer(srcBuffer.Handle());
    src.layout = layout;

    WGPUTextureCopyView dst{};
    dst.texture = Handle::ConvertTexture(Handle());

    WGPUExtent3d extent{};
    extent.width = mWidth;
    extent.height = mHeight;
    extent.depth = 1;

    wgpu_command_encoder_copy_buffer_to_texture(
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
}

Texture::Texture(Texture&& other) : mImpl(std::move(other.mImpl)) {}

Texture::~Texture() {}

void Texture::CopyFrom(const void* data)
{
  mImpl->CopyFrom(data);
}

void Texture::CopyTo(void* data)
{
  mImpl->CopyTo(data);
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
}  // namespace Vortex2D
