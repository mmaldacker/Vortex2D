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

    void InitProlongate(int level,
                        const glm::ivec2& fineSize,
                        Renderer::GenericBuffer& fine, 
                        Renderer::GenericBuffer& fineDiagonal, 
                        Renderer::GenericBuffer& coarse, 
                        Renderer::GenericBuffer& coarseDiagonal);
                        
    void InitRestrict(int level,
                      const glm::ivec2& fineSize,
                      Renderer::GenericBuffer& fine, 
                      Renderer::GenericBuffer& fineDiagonal, 
                      Renderer::GenericBuffer& coarse, 
                      Renderer::GenericBuffer& coarseDiagonal);

    void Prolongate(vk::CommandBuffer commandBuffer, int level);
    void Restrict(vk::CommandBuffer commandBuffer, int level);

private:
    const Renderer::Device& mDevice;
    Renderer::Work mProlongateWork;
    std::vector<Renderer::Work::Bound> mProlongateBound;
    std::vector<Renderer::GenericBuffer*> mProlongateBuffer;

    Renderer::Work mRestrictWork;
    std::vector<Renderer::Work::Bound> mRestrictBound;
    std::vector<Renderer::GenericBuffer*> mRestrictBuffer;
};

}}

#endif
