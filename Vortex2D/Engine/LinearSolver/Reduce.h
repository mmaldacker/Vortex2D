//
//  Reduce.h
//  Vertex2D
//

#ifndef __Vertex2D__Reduce__
#define __Vertex2D__Reduce__

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
    OperatorContext operator()(Buffer &a, Buffer &b);

private:
    std::vector<Buffer> s;
    Operator reduce;
    Operator multiply;
};

}}

#endif /* defined(__Vertex2D__Reduce__) */
