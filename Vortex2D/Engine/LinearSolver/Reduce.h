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

class Reduce
{
public:
    Reduce(glm::vec2 size);

    Context operator()(Buffer &a, Buffer &b);

private:
    std::vector<Buffer> s;
    Operator reduce;
    Operator multiply;
};

}
#endif /* defined(__Vertex2D__Reduce__) */
