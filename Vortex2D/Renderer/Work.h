//
//  Work.h
//  Vortex2D
//

#ifndef Vortex2D_Work_h
#define Vortex2D_Work_h

#include <Vortex2D/Renderer/Buffer.h>
#include <Vortex2D/Renderer/Common.h>
#include <Vortex2D/Renderer/DescriptorSet.h>
#include <Vortex2D/Renderer/Device.h>
#include <Vortex2D/Renderer/Pipeline.h>
#include <Vortex2D/Renderer/Texture.h>

namespace Vortex2D
{
namespace Renderer
{
/**
 * @brief Used for a compute shader, and defines the group size, local size and
 * domain size.
 */
struct ComputeSize
{
  /**
   * @brief The default local size for 2D compute shaders
   * @return a 2d vector
   */
  VORTEX2D_API static glm::ivec2 GetLocalSize2D();

  /**
   * @brief The default local size for 1D compute shaders
   * @return a integer value
   */
  VORTEX2D_API static int GetLocalSize1D();

  /**
   * @brief Computes the 2D group size given a domain size
   * @param size the domain size of the shader
   * @param localSize the local size of the shader
   * @return the group size
   */
  VORTEX2D_API static glm::ivec2 GetWorkSize(const glm::ivec2& size,
                                             const glm::ivec2& localSize = GetLocalSize2D());

  /**
   * @brief Computes the 1D group size given a domain size
   * @param size the domain size of the shader
   * @param localSize the local size of the shader
   * @return the group size
   */
  VORTEX2D_API static glm::ivec2 GetWorkSize(int size, int localSize = GetLocalSize1D());

  /**
   * @brief A default ComputeSize using the default 2D local size. The domain
   * size is (1,1)
   * @return a default compute size
   */
  VORTEX2D_API static ComputeSize Default2D();

  /**
   * @brief A default ComputeSize using the default 1D local size. The domain
   * size is (1,1)
   * @return a default compute size
   */
  VORTEX2D_API static ComputeSize Default1D();

  /**
   * @brief Creates a ComputeSize using a 2D domain size and the default 2D
   * local size.
   * @param size the domain size
   * @param localSize the local size of the shader
   */
  VORTEX2D_API ComputeSize(const glm::ivec2& size, const glm::ivec2& localSize = GetLocalSize2D());

  /**
   * @brief Creates a ComputeSize using a 1D domain size and the default 1D
   * local size.
   * @param size the domain size
   * @param localSize the local size of the shader
   */
  VORTEX2D_API ComputeSize(int size, int localSize = GetLocalSize1D());

  glm::ivec2 DomainSize;
  glm::ivec2 WorkSize;
  glm::ivec2 LocalSize;
};

/**
 * @brief Create a ComputeSize for a stencil type shader
 * @param size the domain size
 * @param radius the stencil size
 * @return calculate ComputeSize
 */
VORTEX2D_API ComputeSize MakeStencilComputeSize(const glm::ivec2& size, int radius);

/**
 * @brief Create a ComputeSize for a checkerboard type shader
 * @param size the domain size
 * @return calculate ComputeSize
 */
VORTEX2D_API ComputeSize MakeCheckerboardComputeSize(const glm::ivec2& size);

/**
 * @brief Parameters for indirect compute: group size, local size, etc
 */
struct DispatchParams
{
  VORTEX2D_API DispatchParams(int count);
  alignas(16) vk::DispatchIndirectCommand workSize;
  alignas(4) uint32_t count;
};

/**
 * @brief Represents a compute shader. It simplifies the process of binding,
 * setting push constants and recording.
 */
class Work
{
public:
  /**
   * @brief Constructs an object using a SPIRV binary. It is not bound to any
   * buffers or textures.
   * @param device vulkan device
   * @param computeSize the compute size. Can be a default one with size (1,1)
   * or one with an actual size.
   * @param spirv binary spirv
   * @param additionalSpecConstInfo additional specialization constants
   */
  VORTEX2D_API Work(const Device& device,
                    const ComputeSize& computeSize,
                    const SpirvBinary& spirv,
                    const SpecConstInfo& additionalSpecConstInfo = {});

  /**
   * @brief Is a bound version of @ref Work. This means a buffer or texture was
   * bound and this can be recorded in a command buffer.
   */
  class Bound
  {
  public:
    VORTEX2D_API Bound();

    /**
     * @brief Adds a constant value, i.e. a push constant.
     * @param commandBuffer the command buffer where the compute work will also
     * be recorded.
     * @param args the data to push. A total of 128 bytes can be used.
     */
    template <typename... Args>
    void PushConstant(vk::CommandBuffer commandBuffer, Args&&... args)
    {
      int offset = mComputeSize.DomainSize.y != 1 ? 8 : 4;
      PushConstantOffset(commandBuffer, offset, std::forward<Args>(args)...);
    }

    /**
     * @brief Record the compute work in this command buffer. This will also set
     * two additional push constants: the 2D domain size.
     * @param commandBuffer the command buffer to record into.
     */
    VORTEX2D_API void Record(vk::CommandBuffer commandBuffer);

    /**
     * @brief Record the compute work in this command buffer. Use the provided
     * parameters to run the compute shader.
     * @param commandBuffer the command buffer to record into.
     * @param dispatchParams the indirect buffer containing the parameters.
     */
    VORTEX2D_API void RecordIndirect(vk::CommandBuffer commandBuffer,
                                     IndirectBuffer<DispatchParams>& dispatchParams);

    friend class Work;

  private:
    Bound(const ComputeSize& computeSize,
          uint32_t pushConstantSize,
          vk::PipelineLayout layout,
          vk::Pipeline pipeline,
          vk::UniqueDescriptorSet descriptor);

    template <typename Arg>
    void PushConstantOffset(vk::CommandBuffer commandBuffer, uint32_t offset, Arg&& arg)
    {
      if (offset + sizeof(Arg) <= mPushConstantSize)
      {
        commandBuffer.pushConstants(
            mLayout, vk::ShaderStageFlagBits::eCompute, offset, sizeof(Arg), &arg);
      }
    }

    template <typename Arg, typename... Args>
    void PushConstantOffset(vk::CommandBuffer commandBuffer,
                            uint32_t offset,
                            Arg&& arg,
                            Args&&... args)
    {
      if (offset + sizeof(Arg) <= mPushConstantSize)
      {
        commandBuffer.pushConstants(
            mLayout, vk::ShaderStageFlagBits::eCompute, offset, sizeof(Arg), &arg);
        PushConstantOffset(commandBuffer, offset + sizeof(Arg), std::forward<Args>(args)...);
      }
    }

    ComputeSize mComputeSize;
    uint32_t mPushConstantSize;
    vk::PipelineLayout mLayout;
    vk::Pipeline mPipeline;
    vk::UniqueDescriptorSet mDescriptor;
  };

  /**
   * @brief Bind the buffers and/or textures.
   * @param inputs a list of buffers and/or textures
   * @return a bound object, ready to be recorded in a command buffer.
   */
  VORTEX2D_API Bound Bind(const std::vector<BindingInput>& inputs);

  /**
   * @brief Bind the buffers and/or textures. This overrides the provided
   * compute size in @ref Work.
   * @param computeSize the compute shader compute size.
   * @param inputs a list of buffers and/or textures
   * @return a bound object, ready to be recorded in a command buffer.
   */
  VORTEX2D_API Bound Bind(ComputeSize computeSize, const std::vector<BindingInput>& inputs);

private:
  ComputeSize mComputeSize;
  const Device& mDevice;
  Renderer::PipelineLayout mPipelineLayout;
  vk::Pipeline mPipeline;
};

}  // namespace Renderer
}  // namespace Vortex2D

#endif
