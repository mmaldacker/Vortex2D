//
//  Texture.h
//  Vortex
//
//  Created by Maximilian Maldacker on 06/04/2014.
//
//

#ifndef __Vortex__Texture__
#define __Vortex__Texture__

#include "Common.h"

namespace Renderer
{

class RenderTexture;

class Texture
{
public:
    enum class PixelFormat
    {
        // 32-bit texture: RGBA8888
        RGBA8888,
        // 32-bit texture: RGB888
        RGB888,
        // 16-bit texture without Alpha channel
        RGB565,
        // 16-bit float texture
        RGBAF,
        // 16-bit float texture
        RGBF,
        // 16-bit float texture
        RGF,
        // 16-bit float texture
        RF,
    };

    Texture() = default;
    Texture(int width, int height, PixelFormat pixelFormat, const void * data = nullptr);
    virtual ~Texture();

    Texture(Texture &&);
    Texture & operator=(Texture &&);

    int Width() const { return mWidth; }
    int Height() const { return mHeight; }

    int StoredWidth() const { return mStoredWidth; }
    int StoredHeight() const { return mStoredHeight; }

    void Bind(int n = 0) const;
    static void Unbind();

    void SetAliasTexParameters();
    void SetAntiAliasTexParameters();

    friend class RenderTexture;
private:
    GLuint mId = 0;
    int mWidth;
    int mHeight;
    int mStoredWidth;
    int mStoredHeight;
    int mOffsetX;
    int mOffsetY;

    static int BoundId[4];
    static int ActiveUnit;
};

}

#endif /* defined(__Vortex__Texture__) */
