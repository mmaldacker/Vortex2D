//
//  Sprite.cpp
//  Vortex
//
//  Created by Maximilian Maldacker on 07/04/2014.
//
//

#include "Sprite.h"

#include <glm/gtc/matrix_transform.hpp>

namespace Renderer
{

Sprite::Sprite(Texture & texture)
    : Colour({1,1,1,1})
    , mColourUniform(Program::ColourTexturePositionProgram(), "u_Colour")
{
    glGenVertexArrays(1,&mVertexArray);
    glBindVertexArray(mVertexArray);

    // tex coords
    glGenBuffers(1, &mTexCoordsBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, mTexCoordsBuffer);

    glVertexAttribPointer(Shader::TexCoords, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glEnableVertexAttribArray(Shader::TexCoords);

    // vertices
    glGenBuffers(1, &mVertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, mVertexBuffer);

    glVertexAttribPointer(Shader::Position, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glEnableVertexAttribArray(Shader::Position);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    Update(texture);
}

Sprite::Sprite(Sprite && other)
{
    mVertexBuffer = other.mVertexBuffer;
    mTexCoordsBuffer = other.mTexCoordsBuffer;
    mVertexArray = other.mVertexArray;

    other.mVertexBuffer = 0;
    other.mTexCoordsBuffer = 0;
    other.mVertexArray = 0;

    mTexture = other.mTexture;
    mColourUniform = other.mColourUniform;

    Colour = other.Colour;

    Update(*mTexture);
}

void Sprite::Update(Texture & texture)
{
    mTexture = &texture;

    float fac_x = (float)mTexture->Width() / mTexture->StoredWidth();
    float fac_y = (float)mTexture->Height() / mTexture->StoredHeight();
    float off_x = (float)mTexture->OffsetX() / mTexture->StoredWidth();
    float off_y = (float)mTexture->OffsetY() / mTexture->StoredHeight();

    float texcoords[] =
    {
        off_x, fac_y + off_y,
        off_x, off_y,
        fac_x + off_x, fac_y + off_y,
        fac_x + off_x, off_y
    };

    float vertices[] =
    {
        0.0, (float)mTexture->Height(),
        0.0, 0.0,
        (float)mTexture->Width(), (float)mTexture->Height(),
        (float)mTexture->Width(), 0.0
    };

    glBindBuffer(GL_ARRAY_BUFFER, mTexCoordsBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(texcoords), texcoords, GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, mVertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Sprite::Render(const glm::mat4 & ortho)
{
    Program::ColourTexturePositionProgram().Use().SetMVP(GetTransform(ortho));
    mColourUniform.Set(Colour);
    mTexture->Bind(0);
    
    glBindVertexArray(mVertexArray);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}

Sprite::~Sprite()
{
    glDeleteBuffers(1, &mVertexBuffer);
    glDeleteBuffers(1, &mTexCoordsBuffer);
    glDeleteVertexArrays(1, &mVertexArray);
}

}
