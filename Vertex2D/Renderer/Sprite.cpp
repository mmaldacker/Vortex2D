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

Sprite::Sprite(Texture & texture) : Sprite(texture,
    {
        {glm::vec2{0.0f}, glm::vec2{(float)texture.Width(), (float)texture.Height()}},
        glm::vec2{0.0f}
    })
{
}

Sprite::Sprite(Texture & texture, const TextureCoords & coords) : Sprite(texture, std::vector<TextureCoords>{coords})
{
}

Sprite::Sprite(Texture & texture, const std::vector<TextureCoords> & rects)
    : Colour({1,1,1,1})
    , mTexture(texture)
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

    Update(rects);
}

Sprite::Sprite(Sprite && other)
    : Transformable(other)
    , Colour(other.Colour)
    , mVertexBuffer(other.mVertexBuffer)
    , mTexCoordsBuffer(other.mTexCoordsBuffer)
    , mVertexArray(other.mVertexArray)
    , mNumTriangles(other.mNumTriangles)
    , mTexture(other.mTexture)
    , mColourUniform(other.mColourUniform)
{
    other.mVertexArray = 0;
}

void Sprite::Coords(const TextureCoords & coords, std::vector<float> & texCoords, std::vector<float> & vertices)
{
    float dx = 1.0f/mTexture.StoredWidth();
    float dy = 1.0f/mTexture.StoredHeight();

    auto rect = coords.rect;
    auto pos = coords.pos;

    glm::vec2 a,b,c,d;

    a.x = rect.Pos.x * dx,                 a.y = (rect.Pos.y + rect.Size.y) * dy;
    b.x = rect.Pos.x * dx,                 b.y = rect.Pos.y * dy;
    c.x = (rect.Pos.x + rect.Size.x) * dx, c.y = (rect.Pos.y + rect.Size.y) * dy;
    d.x = (rect.Pos.x + rect.Size.x) * dx, d.y = rect.Pos.y * dy;

    /*
         b ---- d
         |      |
         |      |
         a ---- c
     
     */

    texCoords.insert(texCoords.end(),
    {
        a.x, a.y,
        c.x, c.y,
        b.x, b.y,

        c.x, c.y,
        d.x, d.y,
        b.x, b.y,
    });


    a.x = pos.x, a.y = pos.y + rect.Size.y;
    b.x = pos.x, b.y = pos.y,
    c.x = pos.x + rect.Size.x, c.y = pos.y + rect.Size.y;
    d.x = pos.x + rect.Size.x, d.y = pos.y;

    vertices.insert(vertices.end(),
    {
        a.x, a.y,
        c.x, c.y,
        b.x, b.y,

        c.x, c.y,
        d.x, d.y,
        b.x, b.y,
    });
}

void Sprite::Update(const std::vector<TextureCoords> & coords)
{
    std::vector<float> texCoords, vertices;
    mNumTriangles = 0;
    for(auto && coord : coords)
    {
        Coords(coord, texCoords, vertices);
        mNumTriangles += 2;
    }

    glBindBuffer(GL_ARRAY_BUFFER, mTexCoordsBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * texCoords.size(), texCoords.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, mVertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertices.size(), vertices.data(), GL_DYNAMIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Sprite::Render(const glm::mat4 & ortho)
{
    Program::ColourTexturePositionProgram().Use().SetMVP(GetTransform(ortho));
    mColourUniform.Set(Colour);
    mTexture.Bind(0);
    
    glBindVertexArray(mVertexArray);
	glDrawArrays(GL_TRIANGLES, 0, mNumTriangles*3);
    glBindVertexArray(0);
}

Sprite::~Sprite()
{
    if(mVertexArray)
    {
        glDeleteBuffers(1, &mVertexBuffer);
        glDeleteBuffers(1, &mTexCoordsBuffer);
        glDeleteVertexArrays(1, &mVertexArray);
    }
}

}