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

    auto operator()(Buffer &a, Buffer &b)
    {
        s[0] = multiply(a, b);

        for(int i = 1 ; i < s.size() ; i++)
        {
            s[i] = reduce(s[i-1]);
        }

        return reduce(s.back());
    }

private:
    std::vector<Buffer> s;
    Operator reduce;
    Operator multiply;
};

}
#endif /* defined(__Vertex2D__Reduce__) */
