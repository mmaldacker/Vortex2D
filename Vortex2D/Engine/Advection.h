//
//  Advection.h
//  Vortex
//

#ifndef Vortex2d_Advection_h
#define Vortex2d_Advection_h

#include <Vortex2D/Renderer/Buffer.h>
#include <Vortex2D/Renderer/Work.h>
#include <Vortex2D/Renderer/CommandBuffer.h>

namespace Vortex2D { namespace Fluid {

class Density;

class Advection
{
public:
    Advection(const Renderer::Device& device, const glm::ivec2& size, float dt, Renderer::Texture& velocity);

    void AdvectVelocity();

    // TODO can only advect one field, need to be able to do as many as we want
    void AdvectInit(Density& density);
    void Advect();

    void AdvectParticleInit(Renderer::GenericBuffer& particles,
                            Renderer::Texture& levelSet,
                            Renderer::GenericBuffer& dispatchParams);
    void AdvectParticles();

private:
    float mDt;
    glm::ivec2 mSize;
    Renderer::Texture& mVelocity;
    // TODO use a common temp velocity texture
    Renderer::Texture mTmpVelocity;

    Renderer::Work mVelocityAdvect;
    Renderer::Work::Bound mVelocityAdvectBound;
    Renderer::Work mAdvect;
    Renderer::Work::Bound mAdvectBound;
    Renderer::Work mAdvectParticles;
    Renderer::Work::Bound mAdvectParticlesBound;

    Renderer::CommandBuffer mAdvectVelocityCmd;
    Renderer::CommandBuffer mAdvectCmd;
    Renderer::CommandBuffer mAdvectParticlesCmd;
};


}}

#endif
