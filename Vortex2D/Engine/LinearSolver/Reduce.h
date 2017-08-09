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
    void Submit();

protected:
    Reduce(const Renderer::Device& device,
           const std::string& fileName,
           const glm::ivec2& size,
           Renderer::Buffer& input,
           Renderer::Buffer& output);

private:
    Renderer::CommandBuffer mCommandBuffer;
    int mSize;
    Renderer::Buffer mReduce;
    vk::UniqueDescriptorSet mDescriptorSet0;
    vk::UniqueDescriptorSet mDescriptorSet1;
    vk::UniquePipelineLayout mPipelineLayout;
    vk::UniquePipeline mPipeline;
};

class ReduceSum : public Reduce
{
public:
    ReduceSum(const Renderer::Device& device,
              const glm::ivec2& size,
              Renderer::Buffer& input,
              Renderer::Buffer& output);
};

class ReduceMax : public Reduce
{
public:
    ReduceMax(const Renderer::Device& device,
              const glm::ivec2& size,
              Renderer::Buffer& input,
              Renderer::Buffer& output);
};

}}

#endif
