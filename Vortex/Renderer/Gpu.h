//
//  Gpu.h
//  Vortex
//

#pragma once

#include <cstdint>

namespace Vortex
{
namespace Renderer
{
enum class MemoryUsage
{
  Gpu,
  Cpu,
  CpuToGpu,
  GpuToCpu,
};

enum class Format
{
  R8Uint,
  R8Sint,
  R32Sfloat,
  R32Sint,
  R8G8B8A8Unorm,
  B8G8R8A8Unorm,
  R32G32Sfloat,
  R32G32B32A32Sfloat,
};

enum class ShaderStage
{
  Vertex,
  Fragment,
  Compute,
};

enum class PrimitiveTopology
{
  Triangle,
  LineList
};

enum class BufferUsage
{
  Vertex,
  Uniform,
  Storage,
  Indirect,
  Index
};

enum class Access
{
  None,
  Write,
  Read,
};

enum class ImageLayout
{
  General,
};

enum class PipelineBindPoint
{
  Graphics,
  Compute
};

enum class BlendFactor
{
  Zero,
  One,
  ConstantColor,
  SrcAlpha,
  OneMinusSrcAlpha,
};

enum class BlendOp
{
  Add,
  Max,
  Min
};

enum class BindType
{
  StorageBuffer,
  StorageImage,
  ImageSampler,
  UniformBuffer,
};

struct DrawIndexedIndirect
{
  std::uint32_t indexCount = {};
  std::uint32_t instanceCount = {};
  std::uint32_t firstIndex = {};
  std::int32_t vertexOffset = {};
  std::uint32_t firstInstance = {};
};

namespace Handle
{
// FIXME doesn't work for 32 bit
typedef struct PipelineLayout_T* PipelineLayout;
typedef struct BindGroupLayout_T* BindGroupLayout;
typedef struct Pipeline_T* Pipeline;
typedef struct ShaderModule_T* ShaderModule;
typedef struct RenderPass_T* RenderPass;
typedef struct Framebuffer_T* Framebuffer;
typedef struct CommandEncoder_T* CommandEncoder;
typedef struct CommandBuffer_T* CommandBuffer;
typedef struct Semaphore_T* Semaphore;
typedef struct BindGroup_T* BindGroup;
typedef struct Buffer_T* Buffer;
typedef struct Sampler_T* Sampler;
typedef struct Image_T* Image;
typedef struct ImageView_T* ImageView;
typedef struct Surface_T* Surface;
}  // namespace Handle

}  // namespace Renderer
}  // namespace Vortex
