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
#include "Quad.h"

#include <vector>

namespace Renderer
{

class Sprite : public Drawable, public Transformable
{
public:
    explicit Sprite(const Texture & texture);
    Sprite(const Texture & texture, Quad && quad);

    Sprite(Sprite &&);

    void Render(const glm::mat4 & ortho);

    glm::vec4 Colour;
    
private:
    Quad mQuad;
    const Texture & mTexture;
    Uniform<glm::vec4> mColourUniform;
};

}

#endif /* defined(__Vortex__Sprite__) */
