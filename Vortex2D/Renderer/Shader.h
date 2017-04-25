//
//  Shader.h
//  Vortex2D
//

#ifndef Vortex_Shader_h
#define Vortex_Shader_h

#include <Vortex2D/Renderer/Common.h>
#include <Vortex2D/Renderer/Device.h>

#include <string>

namespace Vortex2D { namespace Renderer {

/**
 * @brief Represents a Vertex or Fragment shader
 */
class Shader
{
public:
    virtual ~Shader();

    /**
     * @brief Source for the position vertex shader (@see Program)
     */
    static const char* PositionVert;

    /**
     * @brief Source for the position fragment shader (@see Program)
     */
    static const char* PositionFrag;

    /**
     * @brief Source for the texture/position vertex shader (@see Program)
     */
    static const char* TexturePositionVert;

    /**
     * @brief Source for the texture/position fragment shader (@see Program)
     */
    static const char* TexturePositionFrag;


    vk::PipelineShaderStageCreateInfo PipelineInfo;

protected:
    Shader(vk::Device device, const std::string& fileName);

    vk::UniqueShaderModule mShader;
};

/**
 * @brief Vertex Shader
 */
struct VertexShader : Shader
{
    VertexShader(vk::Device device, const std::string& fileName);
};

/**
 * @brief Fragment Shader
 */
struct FragmentShader : Shader
{
    FragmentShader(vk::Device device, const std::string& fileName);
};

/**
 * @brief A program which is defined from a Vertex and Fragment shader
 */
class Program
{
public:
};

class PositionProgram : public Program
{
public:
    PositionProgram(vk::Device device);

    vk::PipelineVertexInputStateCreateInfo VertexInfo;
    vk::PipelineShaderStageCreateInfo ShaderStages[2];

private:
    VertexShader mVertexShader;
    FragmentShader mFragmentShader;
};

class TexturePositionProgram : public Program
{

};

}}

#endif
