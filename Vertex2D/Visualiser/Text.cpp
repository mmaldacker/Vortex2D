//
//  Text.cpp
//  Vertex2D
//
//  Created by Maximilian Maldacker on 20/08/2015.
//  Copyright (c) 2015 Maximilian Maldacker. All rights reserved.
//

#include "Text.h"
#include "Image.h"
#include "ResourcePath.h"

Text::Text()
{
    std::string file = getResourcePath() + "font.png";

    PngLoader image(file);

    mFont = Renderer::Texture(image.Width(),
                              image.Height(),
                              image.HasAlpha() ? Renderer::Texture::PixelFormat::RGBA8888 : Renderer::Texture::PixelFormat::RGB888,
                              image.Data());

}

Renderer::Sprite Text::Render(const std::string &text)
{
    std::vector<Renderer::TextureCoords> coords;

    glm::vec2 size = {mFont.Width() / 16.0f, mFont.Height() / 16.0f };
    glm::vec2 pos = glm::vec2{0.0f};

    for(char c : text)
    {
        int x = c % 16;
        int y = c / 16;

        Renderer::TextureCoords coord;
        coord.tex = {{x*size.x, y*size.y}, size};
        coord.pos = {pos, size};

        coords.push_back(coord);

        pos.x += size.x;
    }

    Renderer::Quad quad{{mFont.Width(), mFont.Height()}, coords};
    return Renderer::Sprite(mFont, std::move(quad));
}

const Renderer::Texture & Text::GetFont() const
{
    return mFont;
}

int Text::GetCharacterHeight() const
{
    return mFont.Height() / 16;
}