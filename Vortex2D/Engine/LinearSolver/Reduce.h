//
//  Reduce.h
//  Vortex2D
//

#ifndef Vortex2D_Reduce_h
#define Vortex2D_Reduce_h

#include <Vortex2D/Renderer/Device.h>
#include <Vortex2D/Renderer/Pipeline.h>
#include <Vortex2D/Renderer/DescriptorSet.h>
#include <Vortex2D/Renderer/Buffer.h>
#include <Vortex2D/Renderer/CommandBuffer.h>

namespace Vortex2D { namespace Fluid {

class Reduce
{
public:
    class Bound
    {
    public:
        void Record(vk::CommandBuffer commandBuffer);

        friend class Reduce;
    private:
        Bound(int size,
              vk::PipelineLayout layout,
              vk::Pipeline pipeline,
              const std::vector<Renderer::Buffer*>& buffers,
              std::vector<vk::UniqueDescriptorSet>&& descriptorSets);

        int mSize;
        vk::PipelineLayout mLayout;
        vk::Pipeline mPipeline;
        std::vector<Renderer::Buffer*> mBuffers;
        std::vector<vk::UniqueDescriptorSet> mDescriptorSets;
    };

    Reduce::Bound Bind(Renderer::Buffer& input, Renderer::Buffer& output);

protected:
    Reduce(const Renderer::Device& device,
           const std::string& fileName,
           const glm::ivec2& size);

private:
    const Renderer::Device& mDevice;
    int mSize;
    std::vector<Renderer::Buffer> mBuffers;
    vk::DescriptorSetLayout mDescriptorLayout;
    vk::UniquePipelineLayout mPipelineLayout;
    vk::UniquePipeline mPipeline;
};

class ReduceSum : public Reduce
{
public:
    ReduceSum(const Renderer::Device& device,
              const glm::ivec2& size);
};

class ReduceMax : public Reduce
{
public:
    ReduceMax(const Renderer::Device& device,
              const glm::ivec2& size);
};

}}

#endif
