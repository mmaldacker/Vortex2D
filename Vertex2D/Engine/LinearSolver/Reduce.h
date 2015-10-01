//
//  Reduce.h
//  Vertex2D
//
//  Created by Maximilian Maldacker on 15/09/2015.
//  Copyright (c) 2015 Maximilian Maldacker. All rights reserved.
//

#ifndef __Vertex2D__Reduce__
#define __Vertex2D__Reduce__

#include "RenderTexture.h"
#include "Shader.h"
#include "Quad.h"
#include "Reader.h"
#include "Operator.h"

namespace Fluid
{

class Reduce
{
public:
    Reduce(glm::vec2 size)
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

    auto operator()(Buffer &a, Buffer &b) -> decltype(std::declval<Operator>()(std::declval<Buffer&>()))
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
        return reduce(s.back());
    }

private:
    std::vector<Buffer> s;
    Operator reduce;
    Renderer::Uniform<glm::vec2> h;
    Operator multiply;
};

}
#endif /* defined(__Vertex2D__Reduce__) */
