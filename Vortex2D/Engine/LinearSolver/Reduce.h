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

class ReduceSum
{
public:
    ReduceSum(const Renderer::Device& device,
              const glm::vec2& size,
              Renderer::Buffer& input,
              Renderer::Buffer& output);

    void Submit();

private:
    Renderer::CommandBuffer mCommandBuffer;
    int mSize;
    Renderer::Buffer mReduce;
    vk::UniqueDescriptorSet mDescriptorSet0;
    vk::UniqueDescriptorSet mDescriptorSet1;
    vk::UniquePipelineLayout mPipelineLayout;
    vk::UniquePipeline mPipeline;
};

class ReduceMax
{
public:
    ReduceMax(const glm::vec2& size);
};

}}

#endif
