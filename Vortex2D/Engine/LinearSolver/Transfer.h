//
//  Transfer.h
//  Vortex2D
//

#ifndef Transfer_h
#define Transfer_h

#include <Vortex2D/Renderer/Buffer.h>
#include <Vortex2D/Renderer/Operator.h>

namespace Vortex2D { namespace Fluid {

class Transfer
{
public:
    Transfer();

    template<typename T1, typename T2>
    Renderer::OperatorContext Prolongate(T1&& input, T2&& pressure)
    {
        return prolongate(std::forward<T1>(input),
                          std::forward<T2>(pressure));
    }

    template<typename T>
    Renderer::OperatorContext Restrict(T&& input)
    {
        return restrict(std::forward<T>(input));
    }

private:
    Renderer::Operator prolongate, restrict;
};

}}

#endif
