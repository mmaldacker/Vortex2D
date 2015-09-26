//
//  RenderTexture.h
//  Vortex
//
//  Created by Maximilian Maldacker on 06/04/2014.
//
//

#ifndef __Vortex__RenderTexture__
#define __Vortex__RenderTexture__

#include "Common.h"
#include "Texture.h"

namespace Renderer
{

class RenderTexture : public Texture
{
public:
    enum class DepthFormat
    {
        NONE = 0,
        DEPTH24 = GL_DEPTH_COMPONENT24,
        DEPTH24_STENCIL8 = GL_DEPTH24_STENCIL8,
    };

    RenderTexture(int width, int height, Texture::PixelFormat pixelFormat, DepthFormat depthFormat = DepthFormat::NONE);
    ~RenderTexture();

    RenderTexture(RenderTexture && other);
    RenderTexture & operator=(RenderTexture && other);

    glm::mat4 Orth;

    void Clear();

    void begin();
    void begin(const glm::vec4 & colour);
    void end();

private:
    GLuint mFrameBuffer;
    GLuint mDepthRenderBuffer;

    GLint mOldViewPort[4];
    GLint mOldFrameBuffer;
};

class PingPong
{
public:
    PingPong(int width, int height, Texture::PixelFormat pixelFormat, RenderTexture::DepthFormat depthFormat = RenderTexture::DepthFormat::NONE);
    PingPong(PingPong && other);

    void Clear();

    void swap();
    void begin();
    void begin(const glm::vec4 & colour);
    void end();

    glm::mat4 Orth;
    RenderTexture Front;
    RenderTexture Back;
};

}

#endif /* defined(__Vortex__RenderTexture__) */
