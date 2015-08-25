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

#include <vector>

namespace Renderer
{

class Sprite : public Drawable, public Transformable
{
public:
    explicit Sprite(Texture & texture);
    Sprite(Texture & texture, const TextureCoords & rect);
    Sprite(Texture & texture, const std::vector<TextureCoords> & rect);
    ~Sprite();

    Sprite(Sprite &&);

    void Render(const glm::mat4 & ortho);

    glm::vec4 Colour;
private:
    void Update(const std::vector<TextureCoords> & coords);
    void Coords(const TextureCoords & coords, std::vector<float> & texCoords, std::vector<float> & vertices);
    
    GLuint mVertexBuffer;
    GLuint mTexCoordsBuffer;
    GLuint mVertexArray;
    int mNumTriangles;

    Texture & mTexture;

    Uniform<glm::vec4> mColourUniform;
};

}

#endif /* defined(__Vortex__Sprite__) */
