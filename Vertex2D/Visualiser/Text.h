//
//  Text.h
//  Vertex2D
//
//  Created by Maximilian Maldacker on 20/08/2015.
//  Copyright (c) 2015 Maximilian Maldacker. All rights reserved.
//

#ifndef __Vertex2D__Text__
#define __Vertex2D__Text__

#include <string>
#include "Texture.h"
#include "Sprite.h"

class Text
{
public:
    Text();

    Renderer::Sprite Render(const std::string & text);

    const Renderer::Texture & GetFont() const;

private:
    Renderer::Texture mFont;
};

#endif /* defined(__Vertex2D__Text__) */
