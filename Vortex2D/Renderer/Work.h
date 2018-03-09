//
//  Work.h
//  Vortex2D
//

#ifndef Vortex2D_Work_h
#define Vortex2D_Work_h

#include <Vortex2D/Renderer/Common.h>
#include <Vortex2D/Renderer/Device.h>
#include <Vortex2D/Renderer/Buffer.h>
#include <Vortex2D/Renderer/Texture.h>
#include <Vortex2D/Renderer/DescriptorSet.h>

namespace Vortex2D { namespace Renderer {

struct ComputeSize
{
    static glm::ivec2 GetLocalSize2D();
    static int GetLocalSize1D();

    static glm::ivec2 GetWorkSize(const glm::ivec2& size);
    static glm::ivec2 GetWorkSize(int size);

    static ComputeSize Default2D();
    static ComputeSize Default1D();

    ComputeSize(const glm::ivec2& size);
    ComputeSize(int size);

    glm::ivec2 DomainSize;
    glm::ivec2 WorkSize;
    glm::ivec2 LocalSize;
};

ComputeSize MakeStencilComputeSize(const glm::ivec2& size, int radius);
ComputeSize MakeCheckerboardComputeSize(const glm::ivec2& size);

struct DispatchParams
{
    DispatchParams(int count);
    alignas(16) vk::DispatchIndirectCommand workSize;
    alignas(4) uint32_t count;
};

class Work
{
public:
    Work(const Device& device,
         const ComputeSize& computeSize,
         const SpirvBinary& spirv);

    class Bound
    {
    public:
        Bound();

        template<typename T>
        void PushConstant(vk::CommandBuffer commandBuffer, uint32_t offset, const T& data)
        {
            if (offset + sizeof(T) <= mPushConstantSize)
            {
                commandBuffer.pushConstants(mLayout, vk::ShaderStageFlagBits::eCompute, offset, sizeof(T), &data);
            }
        }

        void Record(vk::CommandBuffer commandBuffer);
        void RecordIndirect(vk::CommandBuffer commandBuffer, GenericBuffer& dispatchParams);
        void Dispatch(vk::CommandBuffer commandBuffer);

        friend class Work;

    private:
        Bound(const ComputeSize& computeSize,
              uint32_t pushConstantSize,
              vk::PipelineLayout layout,
              vk::Pipeline pipeline,
              vk::UniqueDescriptorSet descriptor);

        ComputeSize mComputeSize;
        uint32_t mPushConstantSize;
        vk::PipelineLayout mLayout;
        vk::Pipeline mPipeline;
        vk::UniqueDescriptorSet mDescriptor;
    };

    Bound Bind(const std::vector<BindingInput>& inputs);
    Bound Bind(ComputeSize computeSize, const std::vector<BindingInput>& inputs);

private:
    ComputeSize mComputeSize;
    const Device& mDevice;
    Renderer::PipelineLayout mPipelineLayout;
    vk::UniquePipeline mPipeline;
};

}}

#endif
