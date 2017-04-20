//
//  Writer.cpp
//  Vortex2D
//

#include "Writer.h"

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

void Writer::Write(const std::vector<uint8_t>& data)
{
    assert(!data.empty());
    Write(data.data());
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
    Write((const void*)data);
}

void Writer::Write(const float* data)
{
    Write((void*)data);
}

void Writer::Write(const void* data)
{
}


}}
