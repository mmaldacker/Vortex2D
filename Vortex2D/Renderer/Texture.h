//
//  Texture.h
//  Vortex2D
//

#ifndef Vortex_Texture_h
#define Vortex_Texture_h

#include "Common.h"

namespace Vortex2D { namespace Renderer {

/**
 * @brief Create and manage a texture
 */
class Texture
{
public:
    enum class PixelFormat
    {
        // 8-bit texture: RGBA8888
        RGBA8888,
        // 8-bit texture: RGB888
        RGB888,
        // 32-bit float texture
        RGBAF,
        // 32-bit float texture
        RGBF,
        // 32-bit float texture
        RGF,
        // 32-bit float texture
        RF,
    };

    Texture() = default;
    Texture(int width, int height, PixelFormat pixelFormat);
    virtual ~Texture();

    Texture(Texture&&);
    Texture & operator=(Texture&&);

    int Width() const { return mWidth; }
    int Height() const { return mHeight; }

    unsigned GetNumberComponents() const;

    void Bind(int n = 0) const;
    static void Unbind();

    void SetAliasTexParameters();
    void SetAntiAliasTexParameters();

    void SetClampToEdgeTexParameters();
    void SetClampToBorderTexParameters();

    friend class Writer;
protected:
    GLint GetInternalFormat() const;
    GLenum GetFormat() const;
    GLenum GetType() const;

    GLuint mId = 0;

private:
    int mWidth;
    int mHeight;
    PixelFormat mFormat;

    static thread_local GLuint BoundId[4];
    static thread_local int ActiveUnit;
};

}}

#endif
