//
//  Transfer.h
//  Vortex2D
//

#include <Vortex2D/Renderer/Buffer.h>
#include <Vortex2D/Renderer/Operator.h>

namespace Vortex2D { namespace Fluid {

class Transfer
{
public:
    Transfer();

    Renderer::OperatorContext Prolongate(Renderer::Buffer& input);
    Renderer::OperatorContext Restrict(Renderer::Buffer& input);

private:
    Renderer::Operator prolongate, restrict;
};

}}
