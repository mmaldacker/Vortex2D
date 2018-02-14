//
//  Reduce.h
//  Vortex2D
//

#ifndef Vortex2D_Reduce_h
#define Vortex2D_Reduce_h

#include <Vortex2D/Renderer/Work.h>
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
        Bound(const std::vector<Renderer::CommandBuffer::CommandFn>& bufferBarriers,
              std::vector<Renderer::Work::Bound>&& bounds);

        std::vector<Renderer::CommandBuffer::CommandFn> mBufferBarriers;
        std::vector<Renderer::Work::Bound> mBounds;
    };

    Reduce::Bound Bind(Renderer::GenericBuffer& input, Renderer::GenericBuffer& output);

protected:
    Reduce(const Renderer::Device& device,
           const Renderer::SpirvBinary& spirv,
           const glm::ivec2& size);

private:
    int mSize;
    Renderer::Work mReduce;
    std::vector<Renderer::Buffer<float>> mBuffers;
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
