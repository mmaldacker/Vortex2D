//
//  Shader.h
//  Vortex2D
//

#ifndef Vortex_Shader_h
#define Vortex_Shader_h

#include <Vortex2D/Renderer/Common.h>
#include <Vortex2D/Renderer/Uniform.h>

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
     * @brief Attribute location of the texture coordinates variable in the shaders
     */
    static const GLuint TexCoords = 1;

    /**
     * @brief Attribute location of the position variable in the shaders
     */
    static const GLuint Position = 2;

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
    Shader(GLuint shader, const char* source);

private:
    GLuint mShader;

    static const char* PositionName;
    static const char* TexCoordsName;
};

/**
 * @brief Vertex Shader
 */
struct VertexShader : Shader
{
    VertexShader(const char* source);
};

/**
 * @brief Fragment Shader
 */
struct FragmentShader : Shader
{
    FragmentShader(const char* source);
};

/**
 * @brief Uniform that hols the location of a uniform in a shader
 * and allows for fast update of it. It is templated on the type of the uniform.
 */
template<typename T>
class Uniform
{
public:
    Uniform() : mLocation(GL_INVALID_VALUE){}
    Uniform(Program& program, const std::string& name);
    void SetLocation(Program& program, const std::string& name);
    void Set(T value);

    friend class Program;
private:
    GLuint mLocation;
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
     * @brief Set the current Program to 0
     */
    static void Unuse();

    /**
     * @brief Set a uniform by name
     */
    template<typename T>
    Program& Set(const std::string& name, T value)
    {
        GLint location = glGetUniformLocation(mProgram, name.c_str());
        assert(location != GL_INVALID_OPERATION);
        glUniform(location, value);

        return *this;
    }

    /**
     * @brief Set the model-view-projection matrix
     * @return returns *this
     */
    Program& SetMVP(const glm::mat4& mvp);

    /**
     * @brief Identity shader with a position & texture component (e.g. used in Sprite)
     */
    static Program& TexturePositionProgram();

    /**
     * @brief Identity shader with a position component and a colour uniform (e.g. used in Shapes)
     */
    static Program& PositionProgram();

    template<typename T>
    friend class Uniform;

protected:
    /**
     * @brief Attach a Fragment or Vertex shader
     * @return returns *this
     */
    Program& AttachShader(const Shader& shader);

    /**
     * @brief Used for setting the variables used in the feedback shader
     */
    Program& AttachFeedback(const std::vector<const GLchar*>& varyings);

    /**
     * @brief Link the shaders to the Program
     */
    Program& Link();

private:
    GLuint mProgram;
    Uniform<glm::mat4> mMVP;

    static thread_local GLuint CurrentProgram;
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
    mLocation = glGetUniformLocation(program.mProgram, name.c_str());
    assert(mLocation != GL_INVALID_OPERATION);
    program.Unuse();
}

template<typename T>
void Uniform<T>::Set(T value)
{
    glUniform(mLocation, value);
}

}}

#endif
