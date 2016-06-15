//
//  Sprite.hpp
//  Vortex2D
//
//  Created by Maximilian Maldacker on 15/06/2016.
//
//

#ifndef Sprite_hpp
#define Sprite_hpp

#include "Drawable.h"
#include "Texture.h"
#include "Shader.h"

namespace Renderer
{

class Sprite : public Drawable
{
public:
    Sprite(const glm::vec2 & size);
    Sprite(Sprite &&);
    ~Sprite();

    void SetTexture(Texture & texture);
    void NoTexture();
    void SetProgram(Program & program);

    void Render(RenderTarget & target, const glm::mat4 & transform = glm::mat4()) override;

private:
    GLuint mVertexArray;
    GLuint mVertexBuffer;

    Texture * mTexture;
    Program * mProgram;
};

}

#endif /* Sprite_hpp */
