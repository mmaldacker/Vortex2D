//
//  Sprite.h
//  Vertex2D
//
//  Created by Maximilian Maldacker on 27/09/2015.
//  Copyright (c) 2015 Maximilian Maldacker. All rights reserved.
//

#ifndef __Vertex2D__Sprite__
#define __Vertex2D__Sprite__

#include "Quad.h"
#include "Texture.h"
#include "Drawable.h"
#include "Transformable.h"

namespace Renderer
{

class Sprite : public Drawable, public Transformable
{
public:
    Sprite(Texture & texture);

    void Render(const glm::mat4 & ortho) override;

private:
    Texture & mTexture;
    Quad mQuad;
};

}

#endif /* defined(__Vertex2D__Sprite__) */
