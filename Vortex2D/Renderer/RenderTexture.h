//
//  RenderTexture.h
//  Vortex2D
//

#ifndef __Vortex__RenderTexture__
#define __Vortex__RenderTexture__

#include "Common.h"
#include "Texture.h"
#include "RenderTarget.h"

namespace Vortex2D { namespace Renderer {

/**
 * @brief Use to draw objects off-screen on a texture.
 */
class RenderTexture : public Texture, public RenderTarget
{
public:
    enum class DepthFormat
    {
        NONE = 0,
        DEPTH24 = GL_DEPTH_COMPONENT24,
        DEPTH24_STENCIL8 = GL_DEPTH24_STENCIL8,
    };

    /**
     * @brief Will create the underlying framebuffer, texture and potential depth/stencil buffer
     * @param width must be greater than 0
     * @param height must be greater than 0
     * @param pixelFormat @see Texture
     * @param depthFormat @see Texture
     */
    RenderTexture(int width, int height, Texture::PixelFormat pixelFormat, DepthFormat depthFormat = DepthFormat::NONE);
    ~RenderTexture();

    RenderTexture(RenderTexture && other);
    RenderTexture & operator=(RenderTexture && other);

    /**
     * @brief Clears the render texture
     */
    void Clear(const glm::vec4 & colour) override;

    /**
     * @brief Render the object to the target
     * @param object An object whose class implements Drawable
     * @param transform An optional aditional transformation matrix to be applied before rendering
     */
    void Render(Drawable & object, const glm::mat4 & transform = glm::mat4()) override;

    /**
     * @brief Clears the stencil buffer to 0
     */
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

}}

#endif /* defined(__Vortex__RenderTexture__) */
