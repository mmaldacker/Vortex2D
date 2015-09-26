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

Reduce::Reduce(Renderer::RenderTexture & r)
    : r(r)
    , t(glm::vec2(r.Width(), r.Height()))
    , mReduceShader("Reduce.vsh", "Reduce.fsh")
    , h(mReduceShader, "h")
    , k(mReduceShader, "k")
{
    glm::vec2 size = t.Size();
    while(size.x > 1.0f && size.y > 1.0f)
    {
        size = glm::ceil(size/glm::vec2(2.0f));

        s.emplace_back(size.x, size.y, Renderer::Texture::PixelFormat::RF);
        q.emplace_back(size);
    }

    mReduceShader.Use().Set("u_texture", 0).Unuse();

    reader.reset(new Renderer::Reader(s.back()));
}

float Reduce::Get()
{
    s[0].begin();
    r.Bind(0);
    mReduceShader.Use().SetMVP(s[0].Orth);
    h.Set(t.Size());
    k.Set(q[0].Size());
    q[0].Render();
    mReduceShader.Unuse();
    s[0].end();

    for(int i = 1 ; i < s.size() ; i++)
    {
        s[i].begin();
        s[i-1].Bind(0);
        mReduceShader.Use().SetMVP(s[i].Orth);
        h.Set(q[i-1].Size());
        k.Set(q[i].Size());
        q[i].Render();
        mReduceShader.Unuse();
        s[i].end();
    }

    return reader->Read().GetFloat(0, 0);
}

}