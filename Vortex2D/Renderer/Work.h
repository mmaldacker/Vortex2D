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

#include <Vortex2D/Utils/variant.hpp>

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

class Work
{
public:
    struct Input
    {
        Input(Renderer::Buffer& buffer);
        Input(Renderer::Texture& texture);
        Input(vk::Sampler sampler, Renderer::Texture& texture);

        struct DescriptorImage
        {
            DescriptorImage(vk::Sampler sampler, Renderer::Texture& texture);
            DescriptorImage(Renderer::Texture& texture);

            vk::Sampler Sampler;
            Renderer::Texture* Texture;
        };

        mpark::variant<Renderer::Buffer*, DescriptorImage> Bind;
    };

    Work(const Device& device,
         const ComputeSize& computeSize,
         const std::string& shader,
         const std::vector<vk::DescriptorType>& bindings,
         const uint32_t pushConstantExtraSize = 0);

    class Bound
    {
    public:
        Bound();

        template<typename T>
        void PushConstant(vk::CommandBuffer commandBuffer, uint32_t offset, const T& data)
        {
            commandBuffer.pushConstants(mLayout, vk::ShaderStageFlagBits::eCompute, offset, sizeof(T), &data);
        }

        void Record(vk::CommandBuffer commandBuffer);

        friend class Work;

    private:
        Bound(const ComputeSize& computeSize,
              vk::PipelineLayout layout,
              vk::Pipeline pipeline,
              vk::UniqueDescriptorSet descriptor);

        ComputeSize mComputeSize;
        vk::PipelineLayout mLayout;
        vk::Pipeline mPipeline;
        vk::UniqueDescriptorSet mDescriptor;
    };

    // TODO save the bound inside the Work class and access with other method
    Bound Bind(const std::vector<Input>& inputs);
    Bound Bind(ComputeSize computeSize, const std::vector<Input>& inputs);

private:
    ComputeSize mComputeSize;
    const Device& mDevice;
    vk::DescriptorSetLayout mDescriptorLayout;
    vk::UniquePipelineLayout mLayout;
    vk::UniquePipeline mPipeline;
    std::vector<vk::DescriptorType> mBindings;
};

}}

#endif
