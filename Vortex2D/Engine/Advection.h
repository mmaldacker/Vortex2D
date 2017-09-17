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

class Advection
{
public:
    Advection(const Renderer::Device& device, const glm::ivec2& size, float dt, Renderer::Texture& velocity);

    void AdvectVelocity();

    // TODO can only advect one field, need to be able to do as many as we want
    // also can only advect RGBA8
    void AdvectInit(Renderer::Texture& field);
    void Advect();

private:
    float mDt;
    Renderer::Texture& mVelocity;
    // TODO use a common temp velocity texture
    Renderer::Texture mTmpVelocity;
    // TODO currently only accepts RGBA8
    Renderer::Texture mField;

    Renderer::Work mVelocityAdvect;
    Renderer::Work::Bound mVelocityAdvectBound;
    Renderer::Work mAdvect;
    Renderer::Work::Bound mAdvectBound;

    Renderer::CommandBuffer mAdvectVelocityCmd;
    Renderer::CommandBuffer mAdvectCmd;
};


}}

#endif
