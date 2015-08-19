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

Quad::Quad(glm::vec2 size)
{
    if (Renderer::supports_npot_textures())
    {
        mStoredSize = mSize;
    }
    else
    {
        mStoredSize.x = Renderer::next_power_of_two((int)mSize.x);
        mStoredSize.y = Renderer::next_power_of_two((int)mSize.y);
    }

    glm::vec4 quad[4];

    // set up 4 vertices to draw over the entire texture (density,velocity, ...)
    quad[0].x = 0.0;     quad[0].y = mSize.y;
    quad[1].x = 0.0;     quad[1].y = 0.0;
    quad[2].x = mSize.x; quad[2].y = mSize.y;
    quad[3].x = mSize.x; quad[3].y = 0.0;

    // set up tex coords
    float dx = 1.0/mStoredSize.x;
    float dy = 1.0/mStoredSize.y;
    quad[0].p = 0.0;                        quad[0].q = dy*(mStoredSize.y-mSize.y);
    quad[1].p = 0.0;                        quad[1].q = 0.0;
    quad[2].p = dx*(mStoredSize.x-mSize.x); quad[2].q = dy*(mStoredSize.y-mSize.y);
    quad[3].p = dx*(mStoredSize.x-mSize.x); quad[3].q = 0.0;

    glGenVertexArrays(1,&mVertexArray);
    glBindVertexArray(mVertexArray);

    glGenBuffers(1, &mVertexBuffer);

    glBindBuffer(GL_ARRAY_BUFFER, mVertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quad), &quad[0][0], GL_STATIC_DRAW);

    // vertex
    glVertexAttribPointer(Renderer::Shader::Position, 2, GL_FLOAT, GL_FALSE, sizeof(quad[0]), 0);
    glEnableVertexAttribArray(Renderer::Shader::Position);

    // tex
    glVertexAttribPointer(Renderer::Shader::TexCoords, 2, GL_FLOAT, GL_FALSE, sizeof(quad[0]), (void*)(sizeof(quad[0][0])*2));
    glEnableVertexAttribArray(Renderer::Shader::TexCoords);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

}

Quad::~Quad()
{
    glDeleteBuffers(1, &mVertexBuffer);
    glDeleteVertexArrays(1, &mVertexArray);
}

void Quad::Render()
{
    glBindVertexArray(mVertexArray);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}

glm::vec2 Quad::Size() const
{
    return mSize;
}

glm::vec2 Quad::FullSize() const
{
    return mStoredSize;
}

}