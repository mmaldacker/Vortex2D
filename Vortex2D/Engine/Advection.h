//
//  Advection.h
//  Vortex
//

#include "Buffer.h"
#include "Operator.h"
#include "Common.h"

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
