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
    std::ifstream is(fileName, std::ios::binary | std::ios::in | std::ios::ate);

    if (!is.is_open())
    {
        throw std::runtime_error("Couldn't open file:" + fileName);
    }

    size_t size = is.tellg();
    is.seekg(0, std::ios::beg);
    std::vector<char> content(size);
    is.read(content.data(), size);
    is.close();

    vk::ShaderModuleCreateInfo shaderInfo;
    shaderInfo
            .setCodeSize(content.size())
            .setPCode((const uint32_t*)content.data());

    mShader = device.createShaderModuleUnique(shaderInfo);
}

Shader::~Shader()
{
}

VertexShader::VertexShader(vk::Device device, const std::string& fileName)
    : Shader(device, fileName)
{
    PipelineInfo
            .setModule(*mShader)
            .setPName("main")
            .setStage(vk::ShaderStageFlagBits::eVertex);
}

FragmentShader::FragmentShader(vk::Device device, const std::string& fileName)
    : Shader(device, fileName)
{
    PipelineInfo
            .setModule(*mShader)
            .setPName("main")
            .setStage(vk::ShaderStageFlagBits::eFragment);
}

PositionProgram::PositionProgram(vk::Device device)
    : mVertexShader(device, "../Vortex2D/Position.vert.spv")
    , mFragmentShader(device, "../Vortex2D/TexturePosition.vert.spv")
{
    vk::VertexInputBindingDescription bindingDescription;
    bindingDescription.setStride(sizeof(glm::vec2));
    bindingDescription.setInputRate(vk::VertexInputRate::eVertex);

    vk::VertexInputAttributeDescription attributeDescription;
    attributeDescription.setFormat(vk::Format::eR32G32Sfloat);

    VertexInfo
            .setVertexBindingDescriptionCount(1)
            .setPVertexBindingDescriptions(&bindingDescription)
            .setVertexAttributeDescriptionCount(1)
            .setPVertexAttributeDescriptions(&attributeDescription);

    ShaderStages[0] = mVertexShader.PipelineInfo;
    ShaderStages[1] = mFragmentShader.PipelineInfo;
}

}}
