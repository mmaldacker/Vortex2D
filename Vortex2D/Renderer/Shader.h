//
//  Shader.h
//  Vortex
//
//  Created by Maximilian Maldacker on 06/04/2014.
//
//

#ifndef __Vortex__Shader__
#define __Vortex__Shader__

#include <string>
#include "Common.h"
#include "Uniform.h"

namespace Renderer
{

class Program;

/**
 * @brief Represents a Vertex or Fragment shader
 */
class Shader
{
public:
    /**
     * @brief Source loads the source of the shader
     * @param source must be a string of the source (and not a filename)
     * @return returns *this
     */
    Shader & Source(const char * source);

    /**
     * @brief Compile compiles the shader
     */
    Shader & Compile();
    virtual ~Shader();

    friend class Program;

    static const GLuint TexCoords = 1;
    static const GLuint Position = 2;

    static const char * PositionVert;
    static const char * PositionFrag;
    static const char * TexturePositionVert;
    static const char * TexturePositionFrag;
protected:
    Shader(GLuint shader);
    GLuint mShader;

private:
    static const char * PositionName;
    static const char * TexCoordsName;
};

/**
 * @brief Vertex Shader
 */
struct VertexShader : Shader
{
    VertexShader();
};

/**
 * @brief Fragment Shader
 */
struct FragmentShader : Shader
{
    FragmentShader();
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
    Uniform(Program & program, const std::string & name);
    void SetLocation(Program & program, const std::string & name);
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
    Program(const char * vertex, const char * fragment);
    ~Program();

    Program(Program &&);
    Program & operator=(Program &&);
    
    Program & AttachShader(const Shader & shader);
    Program & AttachFeedback(const std::vector<const GLchar*> & varyings);
    Program & Link();
    Program & Use();
    static void Unuse();

    template<typename T>
    Program & Set(const std::string & name, T value)
    {
        GLint location = glGetUniformLocation(mProgram, name.c_str());
        assert(location != GL_INVALID_OPERATION);
        glUniform(location, value);

        return *this;
    }

    Program & SetMVP(const glm::mat4 & mvp);

    static Program & TexturePositionProgram();
    static Program & PositionProgram();

    template<typename T>
    friend class Uniform;
private:
    GLuint mProgram;
    Uniform<glm::mat4> mMVP;

    static int CurrentProgram;
};

template<typename T>
Uniform<T>::Uniform(Program & program, const std::string & name)
{
    SetLocation(program, name);
}

template<typename T>
void Uniform<T>::SetLocation(Program & program, const std::string & name)
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

}

#endif /* defined(__Vortex__Shader__) */
