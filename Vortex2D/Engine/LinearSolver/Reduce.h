//
//  Reduce.h
//  Vortex2D
//

#ifndef Vortex2D_Reduce_h
#define Vortex2D_Reduce_h

#include "Operator.h"

namespace Vortex2D { namespace Fluid {

/**
 * @brief An Operator class that implements the inner dot product between two buffers.
 */
class Reduce
{
public:
    Reduce(glm::vec2 size);

    /**
     * @brief Runs the reduce operation
     */
    Renderer::OperatorContext operator()(Renderer::Buffer& a, Renderer::Buffer& b);

private:
    std::vector<Renderer::Buffer> s;
    Renderer::Operator reduce;
    Renderer::Operator multiply;
};

}}

#endif
