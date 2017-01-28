//
//  Writer.cpp
//  Vortex2D
//

#include "Writer.h"
#include "Disable.h"

namespace Vortex2D { namespace Renderer {

Writer::Writer(RenderTexture& texture) : mTexture(texture)
{

}

void Writer::Write(uint8_t* data)
{
    if (mTexture.GetType() != GL_UNSIGNED_BYTE)
    {
        throw std::runtime_error("Cannot write texture");
    }

    Write((void*)data);
}

void Writer::Write(float* data)
{
    if (mTexture.GetType() != GL_FLOAT)
    {
        throw std::runtime_error("Cannot write texture");
    }

    Write((void*)data);
}

void Writer::Write(void* data)
{
    EnableParameter p(glPixelStorei, GL_UNPACK_ALIGNMENT, 1);

    mTexture.Bind();
    glTexImage2D(GL_TEXTURE_2D,
                 0,
                 mTexture.GetInternalFormat(),
                 (GLsizei) mTexture.mWidth,
                 (GLsizei) mTexture.mHeight,
                 0,
                 mTexture.GetFormat(),
                 mTexture.GetType(),
                 data);
    mTexture.Unbind();
}


}}
