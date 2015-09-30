//
//  Reduce.cpp
//  Vertex2D
//
//  Created by Maximilian Maldacker on 15/09/2015.
//  Copyright (c) 2015 Maximilian Maldacker. All rights reserved.
//

#include "Reduce.h"

namespace Fluid
{

Reduce::Reduce(glm::vec2 size)
    : reduce("Reduce.vsh", "Reduce.fsh")
    , h(reduce.Use(), "h")
    , multiply("TexturePosition.vsh", "Multiply.fsh")
{
    while(size.x > 1.0f && size.y > 1.0f)
    {
        s.emplace_back(size, 1);
        size = glm::ceil(size/glm::vec2(2.0f));
    }

    reduce.Use().Set("u_texture", 0).Unuse();
    multiply.Use().Set("u_texture", 0).Set("u_other", 1).Unuse();
}

Context<BindExpr> Reduce::operator()(Buffer &a, Buffer &b)
{
    s[0] = multiply(a, b);

    for(int i = 1 ; i < s.size() ; i++)
    {
        reduce.Use();
        h.Set(s[i-1].size());
        s[i] = reduce(s[i-1]);
    }

    // FIXME this should be included in the expression template
    h.Set(s.back().size());
    return {reduce.Use(), BindExpr{s.back()}};
}

}