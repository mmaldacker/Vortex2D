//
//  Sprite.h
//  Vortex
//
//  Created by Maximilian Maldacker on 07/04/2014.
//
//

#ifndef __Vortex__Sprite__
#define __Vortex__Sprite__

#include "Common.h"
#include "Drawable.h"
#include "Texture.h"
#include "Shader.h"
#include "Transformable.h"

namespace Renderer
{

class Sprite : public Drawable, public Transformable
{
public:
    explicit Sprite(Texture & texture);
    ~Sprite();

    Sprite(Sprite &&);
    Sprite & operator=(Sprite &&);

    Sprite(const Sprite &) = delete;
    Sprite & operator=(const Sprite &) = delete;

    void Update(Texture & texture);
    void Render(const glm::mat4 & ortho);

    glm::vec4 Colour;
private:
    GLuint mVertexBuffer;
    GLuint mTexCoordsBuffer;
    GLuint mVertexArray;

    Texture * mTexture;

    Uniform<glm::vec4> mColourUniform;
};

}

#endif /* defined(__Vortex__Sprite__) */
