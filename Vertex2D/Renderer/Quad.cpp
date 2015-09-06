//
//  Quad.cpp
//  Vertex2D
//
//  Created by Maximilian Maldacker on 19/08/2015.
//  Copyright (c) 2015 Maximilian Maldacker. All rights reserved.
//

#include "Quad.h"
#include "Shader.h"

namespace Renderer
{

void Coords(const glm::vec2 & size, const TextureCoords & coords, std::vector<float> & buffer)
{
    float dx = 1.0f/size.x;
    float dy = 1.0f/size.y;

    auto rect = coords.tex;
    auto pos = coords.pos;

    glm::vec4 a,b,c,d;

    /*
     b ---- d
     |      |
     |      |
     a ---- c

     */

    a.x = rect.Pos.x * dx,                 a.y = (rect.Pos.y + rect.Size.y) * dy;
    b.x = rect.Pos.x * dx,                 b.y = rect.Pos.y * dy;
    c.x = (rect.Pos.x + rect.Size.x) * dx, c.y = (rect.Pos.y + rect.Size.y) * dy;
    d.x = (rect.Pos.x + rect.Size.x) * dx, d.y = rect.Pos.y * dy;

    a.p = pos.Pos.x,              a.q = pos.Pos.y + pos.Size.y;
    b.p = pos.Pos.x,              b.q = pos.Pos.y,
    c.p = pos.Pos.x + pos.Size.x, c.q = pos.Pos.y + pos.Size.y;
    d.p = pos.Pos.x + pos.Size.x, d.q = pos.Pos.y;

    std::vector<float> attributes =
    {
        a.x, a.y, a.p, a.q,
        c.x, c.y, c.p, c.q,
        b.x, b.y, b.p, b.q,

        c.x, c.y, c.p, c.q,
        d.x, d.y, d.p, d.q,
        b.x, b.y, b.p, b.q
    };

    std::copy(attributes.begin(), attributes.end(), std::back_inserter(buffer));
}

Quad::Quad(const glm::vec2 & size) : Quad(size,
    {
        Rect{glm::vec2{0.0f}, size},
        Rect{glm::vec2{0.0f}, size}
    })
{

}

Quad::Quad(const glm::vec2 & size, const TextureCoords & rect) : Quad(size, std::vector<TextureCoords>{rect})
{

}

Quad::Quad(const glm::vec2 & size, const std::vector<TextureCoords> & rect) : mNumTriangles(0)
{
    if (Renderer::supports_npot_textures())
    {
        mSize = size;
    }
    else
    {
        mSize.x = Renderer::next_power_of_two((int)size.x);
        mSize.y = Renderer::next_power_of_two((int)size.y);
    }

    std::vector<float> buffer;
    mNumTriangles = rect.size()*2;

    for(auto && r : rect)
    {
        Coords(mSize, r, buffer);
    }

    glGenVertexArrays(1,&mVertexArray);
    glBindVertexArray(mVertexArray);

    glGenBuffers(1, &mVertexBuffer);

    glBindBuffer(GL_ARRAY_BUFFER, mVertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float)*buffer.size(), buffer.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(Shader::TexCoords, 2, GL_FLOAT, GL_FALSE, 4*sizeof(float), (void*)0);
    glEnableVertexAttribArray(Shader::TexCoords);

    glVertexAttribPointer(Shader::Position, 2, GL_FLOAT, GL_FALSE, 4*sizeof(float), (void*)(sizeof(float)*2));
    glEnableVertexAttribArray(Shader::Position);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

Quad::~Quad()
{
    if(mVertexArray)
    {
        glDeleteBuffers(1, &mVertexBuffer);
        glDeleteVertexArrays(1, &mVertexArray);
    }
}

Quad::Quad(Quad && other)
    : mSize(other.mSize)
    , mVertexArray(other.mVertexArray)
    , mVertexBuffer(other.mVertexBuffer)
    , mNumTriangles(other.mNumTriangles)
{
    other.mVertexArray = 0;
}

void Quad::Render()
{
    glBindVertexArray(mVertexArray);
    glDrawArrays(GL_TRIANGLES, 0, mNumTriangles*3);
    glBindVertexArray(0);
}

const glm::vec2 & Quad::Size() const
{
    return mSize;
}

}