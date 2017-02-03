//
//  Writer.cpp
//  Vortex2D
//

#include "Writer.h"
#include "Disable.h"
#include <glm/gtc/type_ptr.hpp>
#include <stdexcept>
#include <cassert>

namespace Vortex2D { namespace Renderer {

Writer::Writer(Texture& texture)
    : mTexture(texture)
{

}

Writer::Writer(Buffer& buffer)
    : Writer(buffer.mTextures.front())
{

}

void Writer::Write(const std::vector<float>& data)
{
    assert(!data.empty());
    Write(data.data());
}

void Writer::Write(const std::vector<glm::vec2>& data)
{
    assert(!data.empty());
    Write(glm::value_ptr(data[0]));
}

void Writer::Write(const std::vector<glm::vec4>& data)
{
    assert(!data.empty());
    Write(glm::value_ptr(data[0]));
}

void Writer::Write(const uint8_t* data)
{
    if (mTexture.GetType() != GL_UNSIGNED_BYTE)
    {
        throw std::runtime_error("Cannot write texture");
    }

    Write((const void*)data);
}

void Writer::Write(const float* data)
{
    if (mTexture.GetType() != GL_FLOAT)
    {
        throw std::runtime_error("Cannot write texture");
    }

    Write((void*)data);
}

void Writer::Write(const void* data)
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
