//
//  Reduce.h
//  Vertex2D
//
//  Created by Maximilian Maldacker on 15/09/2015.
//  Copyright (c) 2015 Maximilian Maldacker. All rights reserved.
//

#ifndef __Vertex2D__Reduce__
#define __Vertex2D__Reduce__

#include "Operator.h"

namespace Fluid
{

/**
 * @brief An Operator class that implements the inner dot product between two buffers.
 */
class Reduce
{
public:
    Reduce(glm::vec2 size);

    OperatorContext operator()(Buffer &a, Buffer &b);

private:
    std::vector<Buffer> s;
    Operator reduce;
    Operator multiply;
};

}
#endif /* defined(__Vertex2D__Reduce__) */
