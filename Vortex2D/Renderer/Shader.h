//
//  Shader.h
//  Vortex2D
//

#ifndef Vortex_Shader_h
#define Vortex_Shader_h

#include <Vortex2D/Renderer/Common.h>

#include <string>

namespace Vortex2D { namespace Renderer {

class Program;

/**
 * @brief Represents a Vertex or Fragment shader
 */
class Shader
{
public:
    enum class Type
    {
        Vertex,
        Fragment,
        Compute
    };

    virtual ~Shader();

    friend class Program;

    Shader(vk::Device device, const std::string& fileName);

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

private:
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
 * @brief Uniform that hols the location of a uniform in a shader
 * and allows for fast update of it. It is templated on the type of the uniform.
 */
template<typename T>
class Uniform
{
public:
    Uniform() {}
    Uniform(Program& program, const std::string& name);
    void SetLocation(Program& program, const std::string& name);
    void Set(T value);

    friend class Program;
private:
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

    template<typename T>
    friend class Uniform;

protected:
    /**
     * @brief Attach a Fragment or Vertex shader
     * @return returns *this
     */
    Program& AttachShader(const Shader& shader);

    /**
     * @brief Link the shaders to the Program
     */
    Program& Link();

private:
    Uniform<glm::mat4> mMVP;
};

template<typename T>
Uniform<T>::Uniform(Program& program, const std::string& name)
{
    SetLocation(program, name);
}

template<typename T>
void Uniform<T>::SetLocation(Program& program, const std::string& name)
{
    program.Use();
}

template<typename T>
void Uniform<T>::Set(T value)
{
}

}}

#endif
