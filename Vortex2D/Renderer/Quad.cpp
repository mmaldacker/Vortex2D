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

Quad::Quad(const glm::vec2 & size) : mSize(size)
{
    float buffer[] =
    {
        0.0f, 1.0f, 0.0f, size.y,
        0.0f, 0.0f, 0.0f, 0.0f,
        1.0f, 1.0f, size.x, size.y,
        1.0f, 0.0f, size.x, 0.0f
    };

    glGenVertexArrays(1,&mVertexArray);
    glBindVertexArray(mVertexArray);

    glGenBuffers(1, &mVertexBuffer);

    glBindBuffer(GL_ARRAY_BUFFER, mVertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(buffer), buffer, GL_STATIC_DRAW);

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
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}

const glm::vec2 & Quad::Size() const
{
    return mSize;
}

}