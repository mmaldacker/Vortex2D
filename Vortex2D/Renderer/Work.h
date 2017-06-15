//
//  Work.h
//  Vortex2D
//

#ifndef Vortex2D_Work_h
#define Vortex2D_Work_h

#include <Vortex2D/Renderer/Common.h>
#include <Vortex2D/Renderer/Device.h>
#include <Vortex2D/Renderer/Buffer.h>

namespace Vortex2D { namespace Renderer {

class Work
{
public:
    struct Input
    {
        int Binding;
        vk::DescriptorType Type;
        Renderer::Buffer& Buffer;
    };

    Work(const Device& device,
         const glm::vec2& size,
         const std::string& shader,
         const std::vector<Input>& inputs);

    void Dispatch(vk::CommandBuffer commandBuffer);

private:
    int mWidth;
    int mHeight;
    vk::UniqueDescriptorSet mDescriptor;
    vk::UniquePipelineLayout mLayout;
    vk::UniquePipeline mPipeline;
};

}}

#endif
