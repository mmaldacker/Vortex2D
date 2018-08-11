//
//  Cfl.h
//  Vertex2D
//

#ifndef Vertex2D_Cfl_h
#define Vertex2D_Cfl_h

#include <Vortex2D/Engine/LinearSolver/Reduce.h>
#include <Vortex2D/Engine/Velocity.h>
#include <Vortex2D/Renderer/Work.h>
#include <Vortex2D/Renderer/CommandBuffer.h>

namespace Vortex2D { namespace Fluid {

class Cfl
{
public:
    VORTEX2D_API Cfl(const Renderer::Device& device, const glm::ivec2& size, Velocity& velocity);

    VORTEX2D_API void Compute();

    VORTEX2D_API float Get();

private:
    glm::ivec2 mSize;
    Velocity& mVelocity;
    Renderer::Work mVelocityMaxWork;
    Renderer::Work::Bound mVelocityMaxBound;
    Renderer::Buffer<float> mVelocityMax, mCfl;
    Renderer::CommandBuffer mVelocityMaxCmd;
    ReduceMax mReduceVelocityMax;
    ReduceMax::Bound mReduceVelocityMaxBound;
};

}}

#endif
