//
//  Gpu.h
//  Vortex
//

#ifndef Vortex2D_WebGPU_h
#define Vortex2D_WebGPU_h

#include <stdexcept>

#include <Vortex/Renderer/Gpu.h>

extern "C"
{
#include "wgpu.h"
}

namespace Vortex
{
namespace Renderer
{
inline WGPUBufferUsageFlags ConvertBufferUsage(BufferUsage usage)
{
  switch (usage)
  {
    case BufferUsage::Indirect:
      return WGPUBufferUsage_Indirect;
    case BufferUsage::Vertex:
      return WGPUBufferUsage_Vertex;
    case BufferUsage::Uniform:
      return WGPUBufferUsage_Uniform;
    case BufferUsage::Storage:
      return WGPUBufferUsage_Storage;
    case BufferUsage::Index:
      return WGPUBufferUsage_Index;
  }
}

inline WGPUBufferUsageFlags ConvertMemoryUsage(MemoryUsage usage)
{
  switch (usage)
  {
    case MemoryUsage::CpuToGpu:
      return WGPUBufferUsage_MapWrite | WGPUBufferUsage_CopySrc;
    case MemoryUsage::GpuToCpu:
      return WGPUBufferUsage_MapRead | WGPUBufferUsage_CopyDst;
    case MemoryUsage::Gpu:
      return WGPUBufferUsage_CopyDst | WGPUBufferUsage_CopySrc;
    case MemoryUsage::Cpu:
      throw std::invalid_argument("Cpu memory usage not supported");
  }
}

inline WGPUShaderStage ConvertShaderStage(ShaderStage stage)
{
  switch (stage)
  {
    case ShaderStage::Compute:
      return WGPUShaderStage_Compute;
    case ShaderStage::Fragment:
      return WGPUShaderStage_Fragment;
    case ShaderStage::Vertex:
      return WGPUShaderStage_Vertex;
  }
}

inline WGPUTextureFormat ConvertTextureFormat(Format format)
{
  switch (format)
  {
    case Format::R8Uint:
      return WGPUTextureFormat_R8Uint;
    case Format::R8Sint:
      return WGPUTextureFormat_R8Sint;
    case Format::R32Sfloat:
      return WGPUTextureFormat_R32Float;
    case Format::R32Sint:
      return WGPUTextureFormat_R32Sint;
    case Format::R8G8B8A8Unorm:
      return WGPUTextureFormat_RGBA8Unorm;
    case Format::B8G8R8A8Unorm:
      return WGPUTextureFormat_BGRA8Unorm;
    case Format::R32G32Sfloat:
      return WGPUTextureFormat_RG32Float;
    case Format::R32G32B32A32Sfloat:
      return WGPUTextureFormat_RGBA32Float;
  }
}

inline WGPUTextureUsageFlags ConvertTextureUsage(MemoryUsage memoryUsage)
{
  switch (memoryUsage)
  {
    case MemoryUsage::Gpu:
      return WGPUTextureUsage_CopyDst | WGPUTextureUsage_CopySrc |
             WGPUTextureUsage_RenderAttachment | WGPUTextureUsage_Sampled |
             WGPUTextureUsage_Storage;
    case MemoryUsage::CpuToGpu:
    case MemoryUsage::GpuToCpu:
      return WGPUTextureUsage_CopyDst | WGPUTextureUsage_CopySrc | WGPUTextureUsage_Storage;
    case MemoryUsage::Cpu:
      throw std::invalid_argument("Cpu memory usage not supported");
  }
}

namespace Handle
{
inline WGPUBuffer ConvertBuffer(Buffer buffer)
{
  return reinterpret_cast<WGPUBuffer>(buffer);
}
inline WGPUCommandEncoder ConvertCommandEncoder(CommandEncoder commandEncoder)
{
  return reinterpret_cast<WGPUCommandEncoder>(commandEncoder);
}
inline WGPUCommandBuffer ConvertCommandBuffer(CommandBuffer commandBuffer)
{
  return reinterpret_cast<WGPUCommandBuffer>(commandBuffer);
}
inline WGPUTexture ConvertImage(Image image)
{
  return reinterpret_cast<WGPUTexture>(image);
}
inline WGPUBindGroupLayout ConvertBindGroupLayout(BindGroupLayout layout)
{
  return reinterpret_cast<WGPUBindGroupLayout>(layout);
}
inline WGPUTexture ConvertTexture(Image image)
{
  return reinterpret_cast<WGPUTexture>(image);
}
}  // namespace Handle
}  // namespace Renderer
}  // namespace Vortex

#endif
