//
//  Writer.h
//  Vortex2D
//

#ifndef Vortex2D_Writer_h
#define Vortex2D_Writer_h

#include "Common.h"
#include "Texture.h"
#include "Buffer.h"

namespace Vortex2D { namespace Renderer {

/**
 * @brief Helper class to write data to a texture
 */
class Writer
{
public:
    Writer(Texture& texture);
    Writer(Buffer& buffer);

    void Write(uint8_t* data);
    void Write(float* data);

private:
    void Write(void* data);

    Texture& mTexture;
};

}}

#endif
