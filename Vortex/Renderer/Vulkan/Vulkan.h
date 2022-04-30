//
//  Gpu.h
//  Vortex
//

#pragma once

#include <Vortex/Renderer/Gpu.h>

#define VULKAN_HPP_TYPESAFE_CONVERSION 1
#include <vulkan/vulkan.hpp>

#define VMA_RECORDING_ENABLED 0

#include "vk_mem_alloc.h"

#define VK_LAYER_LUNARG_STANDARD_VALIDATION_NAME "VK_LAYER_LUNARG_standard_validation"

namespace Vortex
{
namespace Renderer
{
inline bool HasLayer(const char* extension,
                     const std::vector<vk::LayerProperties>& availableExtensions)
{
  return std::any_of(availableExtensions.begin(),
                     availableExtensions.end(),
                     [&](const vk::LayerProperties& layer)
                     { return std::strcmp(extension, layer.layerName) == 0; });
}

inline bool HasExtension(const char* extension,
                         const std::vector<vk::ExtensionProperties>& availableExtensions)
{
  return std::any_of(availableExtensions.begin(),
                     availableExtensions.end(),
                     [&](const vk::ExtensionProperties& layer)
                     { return std::strcmp(extension, layer.extensionName) == 0; });
}

inline VmaMemoryUsage ConvertMemoryUsage(MemoryUsage memoryUsage)
{
  switch (memoryUsage)
  {
    case MemoryUsage::Cpu:
      return VMA_MEMORY_USAGE_CPU_ONLY;
    case MemoryUsage::Gpu:
      return VMA_MEMORY_USAGE_GPU_ONLY;
    case MemoryUsage::GpuToCpu:
      return VMA_MEMORY_USAGE_GPU_TO_CPU;
    case MemoryUsage::CpuToGpu:
      return VMA_MEMORY_USAGE_CPU_TO_GPU;
  }
}

inline vk::Format ConvertFormat(Format format)
{
  switch (format)
  {
    case Format::R8Uint:
      return vk::Format::eR8Uint;
    case Format::R8Sint:
      return vk::Format::eR8Sint;
    case Format::R32Sfloat:
      return vk::Format::eR32Sfloat;
    case Format::R32Sint:
      return vk::Format::eR32Sint;
    case Format::R8G8B8A8Unorm:
      return vk::Format::eR8G8B8A8Unorm;
    case Format::B8G8R8A8Unorm:
      return vk::Format::eB8G8R8A8Unorm;
    case Format::R32G32Sfloat:
      return vk::Format::eR32G32Sfloat;
    case Format::R32G32B32A32Sfloat:
      return vk::Format::eR32G32B32A32Sfloat;
  }
}

inline vk::ShaderStageFlagBits ConvertShaderStage(ShaderStage shaderStage)
{
  switch (shaderStage)
  {
    case ShaderStage::Vertex:
      return vk::ShaderStageFlagBits::eVertex;
    case ShaderStage::Fragment:
      return vk::ShaderStageFlagBits::eFragment;
    case ShaderStage::Compute:
      return vk::ShaderStageFlagBits::eCompute;
  }
}

inline vk::PrimitiveTopology ConvertTopology(PrimitiveTopology topology)
{
  switch (topology)
  {
    case PrimitiveTopology::Triangle:
      return vk::PrimitiveTopology::eTriangleList;
    case PrimitiveTopology::LineList:
      return vk::PrimitiveTopology::eLineList;
  }
}

inline vk::BufferUsageFlags ConvertBufferUsage(BufferUsage bufferUsage)
{
  vk::BufferUsageFlags bufferUsageFlags;
  switch (bufferUsage)
  {
    case BufferUsage::Indirect:
      bufferUsageFlags =
          vk::BufferUsageFlagBits::eIndirectBuffer | vk::BufferUsageFlagBits::eStorageBuffer;
      break;
    case BufferUsage::Vertex:
      bufferUsageFlags =
          vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eVertexBuffer;
      break;
    case BufferUsage::Uniform:
      bufferUsageFlags = vk::BufferUsageFlagBits::eUniformBuffer;
      break;
    case BufferUsage::Storage:
      bufferUsageFlags = vk::BufferUsageFlagBits::eStorageBuffer;
      break;
    case BufferUsage::Index:
      bufferUsageFlags = vk::BufferUsageFlagBits::eIndexBuffer;
      break;
  }

  return bufferUsageFlags | vk::BufferUsageFlagBits::eTransferDst |
         vk::BufferUsageFlagBits::eTransferSrc;
}

inline vk::ImageLayout ConvertImageLayout(ImageLayout layout)
{
  switch (layout)
  {
    case ImageLayout::General:
      return vk::ImageLayout::eGeneral;
  }
}

inline vk::AccessFlags ConvertAccess(Access access)
{
  switch (access)
  {
    case Access::None:
      return {};
    case Access::Read:
      return vk::AccessFlagBits::eShaderRead | vk::AccessFlagBits::eColorAttachmentRead;
    case Access::Write:
      return vk::AccessFlagBits::eShaderWrite | vk::AccessFlagBits::eColorAttachmentWrite;
  }
}

inline vk::PipelineBindPoint ConvertPipelineBindPoint(PipelineBindPoint bindPoint)
{
  switch (bindPoint)
  {
    case PipelineBindPoint::Compute:
      return vk::PipelineBindPoint::eCompute;
    case PipelineBindPoint::Graphics:
      return vk::PipelineBindPoint::eGraphics;
  }
}

inline vk::BlendFactor ConvertBlendFactor(BlendFactor factor)
{
  switch (factor)
  {
    case BlendFactor::One:
      return vk::BlendFactor::eOne;
    case BlendFactor::Zero:
      return vk::BlendFactor::eZero;
    case BlendFactor::SrcAlpha:
      return vk::BlendFactor::eSrcAlpha;
    case BlendFactor::OneMinusSrcAlpha:
      return vk::BlendFactor::eOneMinusSrcAlpha;
    case BlendFactor::ConstantColor:
      return vk::BlendFactor::eConstantColor;
  }
}

inline vk::BlendOp ConvertBlendOp(BlendOp blendOp)
{
  switch (blendOp)
  {
    case BlendOp::Add:
      return vk::BlendOp::eAdd;
    case BlendOp::Max:
      return vk::BlendOp::eMax;
    case BlendOp::Min:
      return vk::BlendOp::eMin;
  }
}

inline vk::DescriptorType ConvertDescriptorType(BindType type)
{
  switch (type)
  {
    case BindType::StorageBuffer:
      return vk::DescriptorType::eStorageBuffer;
    case BindType::StorageImage:
      return vk::DescriptorType::eStorageImage;
    case BindType::ImageSampler:
      return vk::DescriptorType::eCombinedImageSampler;
    case BindType::UniformBuffer:
      return vk::DescriptorType::eUniformBuffer;
  }
}

namespace Handle
{
inline Semaphore ConvertSemaphore(vk::Semaphore semaphore)
{
  return reinterpret_cast<Semaphore>(static_cast<VkSemaphore>(semaphore));
}

inline vk::Semaphore ConvertSemaphore(Semaphore semaphore)
{
  return reinterpret_cast<VkSemaphore>(semaphore);
}

inline Framebuffer ConvertFramebuffer(vk::Framebuffer framebuffer)
{
  return reinterpret_cast<Framebuffer>(static_cast<VkFramebuffer>(framebuffer));
}

inline vk::CommandBuffer ConvertCommandBuffer(CommandBuffer commandBuffer)
{
  return reinterpret_cast<VkCommandBuffer>(commandBuffer);
}

inline vk::Image ConvertImage(Image image)
{
  return reinterpret_cast<VkImage>(image);
}

inline Image ConvertImage(vk::Image image)
{
  return reinterpret_cast<Image>(static_cast<VkImage>(image));
}

inline vk::ImageView ConvertImageView(ImageView imageView)
{
  return reinterpret_cast<VkImageView>(imageView);
}

inline ImageView ConvertImageView(vk::ImageView imageView)
{
  return reinterpret_cast<ImageView>(static_cast<VkImageView>(imageView));
}

inline vk::Buffer ConvertBuffer(Buffer buffer)
{
  return reinterpret_cast<VkBuffer>(buffer);
}

inline vk::Sampler ConvertSampler(Sampler sampler)
{
  return reinterpret_cast<VkSampler>(sampler);
}

inline vk::SurfaceKHR ConvertSurface(Surface surface)
{
  return reinterpret_cast<VkSurfaceKHR>(surface);
}

inline Surface ConvertSurface(vk::SurfaceKHR surface)
{
  return reinterpret_cast<Surface>(static_cast<VkSurfaceKHR>(surface));
}

}  // namespace Handle
}  // namespace Renderer
}  // namespace Vortex
