//
//  Texture.h
//  Vortex2D
//

#ifndef Vortex_Texture_h
#define Vortex_Texture_h

#include <Vortex2D/Renderer/Common.h>

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

    unsigned GetNumberComponents() const;

    void Bind(int n = 0) const;

    void Nearest();
    void Linear();
    void BorderColour(const glm::vec4& colour);

    void ClampToEdge();
    void ClampToBorder();

    friend class Writer;

private:
    PixelFormat mFormat;
};

}}

#endif
