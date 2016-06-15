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
#include "RenderTarget.h"

namespace Renderer
{

class RenderTexture : public Texture, public RenderTarget
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

    void Clear(const glm::vec4 & colour) override;
    void Render(Drawable & object, const glm::mat4 & transform = glm::mat4()) override;

    void ClearStencil();

    friend class Reader;
private:
    void Begin();
    void End();

    GLuint mFrameBuffer;
    GLuint mDepthRenderBuffer;

    GLint mOldViewPort[4];
    GLint mOldFrameBuffer;
};

}

#endif /* defined(__Vortex__RenderTexture__) */
