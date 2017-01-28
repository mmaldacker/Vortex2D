//
//  Writer.h
//  Vortex2D
//

#ifndef Vortex2D_Writer_h
#define Vortex2D_Writer_h

#include "Common.h"
#include "RenderTexture.h"

namespace Vortex2D { namespace Renderer {

class Writer
{
public:
    Writer(RenderTexture& texture);

    void Write(uint8_t* data);
    void Write(float* data);

private:
    void Write(void* data);

    RenderTexture& mTexture;
};

}}

#endif