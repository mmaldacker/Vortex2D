//
//  Shader.cpp
//  Vortex2D
//

#include "Shader.h"

#include <stdexcept>
#include <cassert>
#include <string>
#include <fstream>
#include <streambuf>

namespace Vortex2D { namespace Renderer {

const char* Shader::PositionVert;
const char* Shader::PositionFrag;
const char* Shader::TexturePositionVert;
const char* Shader::TexturePositionFrag;


Shader::Shader(vk::Device device, const std::string& fileName)
{
    std::ifstream input(fileName);
    std::string content((std::istreambuf_iterator<char>(input)),
                        std::istreambuf_iterator<char>());

    std::vector<uint32_t> contentAligned(content.size() / sizeof(uint32_t) + 1);
    std::memcpy(contentAligned.data(), content.data(), content.size());

    vk::ShaderModuleCreateInfo shaderInfo;
    shaderInfo
            .setCodeSize(contentAligned.size())
            .setPCode(contentAligned.data());

    mShader = device.createShaderModuleUnique(shaderInfo);
}

Shader::~Shader()
{
}

VertexShader::VertexShader(vk::Device device, const std::string& fileName)
    : Shader(device, fileName)
{
}

FragmentShader::FragmentShader(vk::Device device, const std::string& fileName)
    : Shader(device, fileName)
{
}

Program::Program(const char* vertexSource, const char* fragmentSource)
{
}

Program::Program()
{
}

Program::~Program()
{
}

Program::Program(Program&& other)
{
}

Program & Program::operator=(Program&& other)
{
    return *this;
}

Program & Program::AttachShader(const Shader& shader)
{
    return *this;
}

Program & Program::Link()
{
    return *this;
}

Program& Program::Use()
{
    return *this;
}

Program& Program::SetMVP(const glm::mat4& mvp)
{
    return *this;
}

}}
