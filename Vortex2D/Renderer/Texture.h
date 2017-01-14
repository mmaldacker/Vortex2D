//
//  Texture.h
//  Vortex2D
//

#ifndef __Vortex__Texture__
#define __Vortex__Texture__

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
    Texture(int width, int height, PixelFormat pixelFormat, const void* data = nullptr);
    virtual ~Texture();

    Texture(Texture&&);
    Texture & operator=(Texture&&);

    int Width() const { return mWidth; }
    int Height() const { return mHeight; }

    void Bind(int n = 0) const;
    static void Unbind();

    void SetAliasTexParameters();
    void SetAntiAliasTexParameters();

    void SetClampToEdgeTexParameters();
    void SetClampToBorderTexParameters();

    PixelFormat GetFormat() const;

protected:
    GLuint mId = 0;

private:
    int mWidth;
    int mHeight;
    PixelFormat mFormat;

    static GLuint BoundId[4];
    static int ActiveUnit;
};

}}

#endif /* defined(__Vortex__Texture__) */
