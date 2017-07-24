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

namespace Vortex2D { namespace Renderer {

class Work
{
public:
    struct Input
    {
        Input(Renderer::Buffer& buffer);
        Input(Renderer::Texture& texture);

        union
        {
            Renderer::Buffer* Buffer;
            Renderer::Texture* Texture;
        };
    };

    Work(const Device& device,
         const glm::vec2& size,
         const std::string& shader,
         const std::vector<vk::DescriptorType>& bindings);

    class Bound
    {
    public:
        Bound() = default;

        template<typename T>
        void PushConstant(vk::CommandBuffer commandBuffer, uint32_t offset, const T& data)
        {
            commandBuffer.pushConstants(mLayout, vk::ShaderStageFlagBits::eCompute, offset, sizeof(T), &data);
        }

        void Record(vk::CommandBuffer commandBuffer);

        friend class Work;

    private:
        Bound(uint32_t widht,
              uint32_t height,
              vk::PipelineLayout layout,
              vk::Pipeline pipeline,
              vk::UniqueDescriptorSet descriptor);

        uint32_t mWidth, mHeight;
        vk::PipelineLayout mLayout;
        vk::Pipeline mPipeline;
        vk::UniqueDescriptorSet mDescriptor;
    };

    Bound Bind(const std::vector<Input>& inputs);

private:
    uint32_t mWidth, mHeight;
    const Device& mDevice;
    vk::DescriptorSetLayout mDescriptorLayout;
    vk::UniquePipelineLayout mLayout;
    vk::UniquePipeline mPipeline;
    std::vector<vk::DescriptorType> mBindings;
};

}}

#endif
