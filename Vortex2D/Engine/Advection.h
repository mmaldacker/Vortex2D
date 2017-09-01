//
//  Advection.h
//  Vortex
//

#include <Vortex2D/Renderer/Buffer.h>
#include <Vortex2D/Renderer/Work.h>
#include <Vortex2D/Renderer/CommandBuffer.h>

namespace Vortex2D { namespace Fluid {

class Advection
{
public:
    Advection(const Renderer::Device& device, const glm::ivec2& size, float dt, Renderer::Texture& velocity);

    void Advect();
    void Advect(Renderer::Buffer& buffer);

private:
    // TODO use a common temp velocity texture
    Renderer::Texture mVelocity;
    Renderer::Work mVelocityAdvect;
    Renderer::Work::Bound mVelocityAdvectBound;
    Renderer::Work mAdvect;
    Renderer::Work::Bound mAdvectBound;

    Renderer::CommandBuffer mAdvecVelocityCmd;
};


}}
