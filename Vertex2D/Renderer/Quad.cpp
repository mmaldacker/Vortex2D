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

void Coords(const glm::vec2 & size, std::vector<float> & buffer)
{
    glm::vec4 a,b,c,d;

    /*
     b ---- d
     |      |
     |      |
     a ---- c

     */

    a.x = 0.0f, a.y = 1.0f;
    b.x = 0.0f, b.y = 0.0f;
    c.x = 1.0f, c.y = 1.0f;
    d.x = 1.0f, d.y = 0.0f;

    a.p = 0.0f,   a.q = size.y;
    b.p = 0.0f,   b.q = 0.0f,
    c.p = size.x, c.q = size.y;
    d.p = size.x, d.q = 0.0f;

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

Quad::Quad(const glm::vec2 & size)
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
    Coords(mSize, buffer);

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
{
    other.mVertexArray = 0;
}

void Quad::Render()
{
    glBindVertexArray(mVertexArray);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}

const glm::vec2 & Quad::Size() const
{
    return mSize;
}

}