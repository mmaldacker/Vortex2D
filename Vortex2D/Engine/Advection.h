//
//  Advection.h
//  Vortex
//

#include <Vortex2D/Renderer/Common.h>
#include <Vortex2D/Renderer/Data.h>
#include <Vortex2D/Renderer/Operator.h>

namespace Vortex2D { namespace Fluid {

class Advection
{
public:
    Advection(float dt, Renderer::Buffer& velocity);

    void Advect();
    void Advect(Renderer::Buffer& buffer);

private:
    Renderer::Buffer& mVelocity;

    Renderer::Operator mVelocityAdvect;
    Renderer::Operator mAdvect;
};


}}
