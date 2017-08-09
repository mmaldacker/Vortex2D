//
//  Transfer.h
//  Vortex2D
//

#ifndef Transfer_h
#define Transfer_h

#include <Vortex2D/Renderer/Work.h>
#include <Vortex2D/Renderer/CommandBuffer.h>

namespace Vortex2D { namespace Fluid {

class Transfer
{
public:
    Transfer(const Renderer::Device& device);

    void Init(const glm::vec2& coarseSize, Renderer::Buffer& coarse, Renderer::Buffer& fine);

    void Prolongate(int level);
    void Restrict(int level);

private:
    const Renderer::Device& mDevice;
    Renderer::Work mProlongateWork;
    std::vector<Renderer::Work::Bound> mProlongateBound;
    std::vector<Renderer::CommandBuffer> mProlongateCmd;

    Renderer::Work mRestrictWork;
    std::vector<Renderer::Work::Bound> mRestrictBound;
    std::vector<Renderer::CommandBuffer> mRestrictCmd;
};

}}

#endif
