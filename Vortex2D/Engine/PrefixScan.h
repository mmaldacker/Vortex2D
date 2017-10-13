//
//  PrefixScan.h
//  Vortex2D
//

#ifndef Vortex2D_PrefixScan_h
#define Vortex2D_PrefixScan_h

#include <Vortex2D/Renderer/Work.h>
#include <Vortex2D/Renderer/CommandBuffer.h>

namespace Vortex2D { namespace Fluid {

class PrefixScan
{
public:
    class Bound
    {
    public:
        void Record(vk::CommandBuffer commandBuffer);

        friend class PrefixScan;
    private:
        Bound(const std::vector<Renderer::CommandBuffer::CommandFn>& bufferBarriers,
              std::vector<Renderer::Work::Bound>&& bounds);

        std::vector<Renderer::CommandBuffer::CommandFn> mBufferBarriers;
        std::vector<Renderer::Work::Bound> mBounds;
    };

    PrefixScan(const Renderer::Device& device, const glm::ivec2& size);

    Bound Bind(Renderer::GenericBuffer& input, Renderer::GenericBuffer& output, Renderer::GenericBuffer& dispatchParams);

private:
    void BindRecursive(std::vector<Renderer::CommandBuffer::CommandFn>& bufferBarriers,
                       std::vector<Renderer::Work::Bound>& bound,
                       Renderer::GenericBuffer& input,
                       Renderer::GenericBuffer& output,
                       Renderer::GenericBuffer& dispatchParams,
                       Renderer::ComputeSize computeSize,
                       int level);

    int mSize;
    Renderer::Work mAddWork;
    Renderer::Work mPreScanWork;
    Renderer::Work mPreScanStoreSumWork;

    std::vector<Renderer::GenericBuffer> mPartialSums;
};

}}

#endif
