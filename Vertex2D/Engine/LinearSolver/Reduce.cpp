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
    : mReduce("Reduce.vsh", "Reduce.fsh")
    , h(mReduce.Use(), "h")
    , k(mReduce.Use(), "k")
    , multiply("TexturePosition.vsh", "Multiply.fsh")
{
    s.emplace_back(size.x, size.y, Renderer::Texture::PixelFormat::RF);
    q.emplace_back(size);

    while(size.x > 1.0f && size.y > 1.0f)
    {
        size = glm::ceil(size/glm::vec2(2.0f));
        s.emplace_back(size.x, size.y, Renderer::Texture::PixelFormat::RF);
        q.emplace_back(size);
    }

    mReduce.Use().Set("u_texture", 0).Unuse();
    multiply.Use().Set("u_texture", 0).Set("u_other", 1).Unuse();

    reader.reset(new Renderer::Reader(s.back()));
}

void Reduce::apply(Renderer::RenderTexture &a, Renderer::RenderTexture &b)
{
    multiply.apply(q[0], s[0], a, b);

    for(int i = 1 ; i < s.size() ; i++)
    {
        mReduce.Use();
        h.Set(q[i-1].Size());
        k.Set(q[i].Size());

        mReduce.apply(q[i], s[i], s[i-1]);
    }
}

float Reduce::Get()
{
    return reader->Read().GetFloat(0, 0);
}

void Reduce::Bind(int n)
{
    s.back().Bind(n);
}

}