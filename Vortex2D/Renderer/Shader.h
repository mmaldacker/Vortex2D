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

class Program;

/**
 * @brief Represents a Vertex or Fragment shader
 */
class Shader
{
public:
    virtual ~Shader();

    friend class Program;


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

protected:
    Shader(vk::Device device, const std::string& fileName);

    vk::UniqueShaderModule mShader;
    vk::PipelineShaderStageCreateInfo mPipelineInfo;
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
    Program();
    Program(const char* vertex, const char* fragment);
    ~Program();

    Program(Program&&);
    Program& operator=(Program&&);

    /**
     * @brief Use this Program
     */
    Program& Use();

    /**
     * @brief Set a uniform by name
     */
    template<typename T>
    Program& Set(const std::string& name, T value)
    {
        return *this;
    }

    /**
     * @brief Set the model-view-projection matrix
     * @return returns *this
     */
    Program& SetMVP(const glm::mat4& mvp);
};


}}

#endif
