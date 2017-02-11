//
//  Shader.cpp
//  Vortex2D
//

#include "Shader.h"

#include <stdexcept>
#include <cassert>

namespace Vortex2D { namespace Renderer {

const char * Shader::PositionName = "a_Position";
const char * Shader::TexCoordsName = "a_TexCoords";

const char * Shader::PositionVert = GLSL(
    in vec2 a_Position;
    uniform mat4 u_Projection;

    void main()
    {
        gl_Position = u_Projection * vec4(a_Position, 0.0, 1.0);
    }
);

const char * Shader::PositionFrag = GLSL(
    uniform vec4 u_Colour;
    out vec4 out_color;

    void main()
    {
        out_color = u_Colour;
    }
);

const char * Shader::TexturePositionVert = GLSL(
    in vec2 a_Position;
    in vec2 a_TexCoords;

    out vec2 v_texCoord;

    uniform mat4 u_Projection;

    void main()
    {
        gl_Position = u_Projection * vec4(a_Position, 0.0, 1.0);
        v_texCoord = a_TexCoords;
    }
);

const char * Shader::TexturePositionFrag = GLSL(
    in vec2 v_texCoord;
    uniform sampler2D u_texture;

    out vec4 out_color;

    void main()
    {
        out_color = texture(u_texture, v_texCoord);
    }
);

Shader::Shader(GLuint shader, const char* source) : mShader(shader)
{
    glShaderSource(mShader, 1, &source, NULL);

    glCompileShader(mShader);

    GLint status;
    glGetShaderiv(mShader, GL_COMPILE_STATUS, &status);

    if(!status)
    {
        GLsizei length;
        glGetShaderiv(mShader, GL_INFO_LOG_LENGTH, &length);
        std::vector<GLchar> src(length);

        glGetShaderInfoLog(mShader, length, NULL, src.data());
        throw std::runtime_error("Error compiling shader: \n" + std::string(src.data()));
    }

}

Shader::~Shader()
{
    assert(mShader);
    glDeleteShader(mShader);
}

VertexShader::VertexShader(const char* source)
    : Shader(glCreateShader(GL_VERTEX_SHADER), source)
{
}

FragmentShader::FragmentShader(const char* source)
    : Shader(glCreateShader(GL_FRAGMENT_SHADER), source)
{
}

thread_local GLuint Program::CurrentProgram = 0;

Program::Program(const char* vertexSource, const char* fragmentSource)
    : mProgram(glCreateProgram())
{
    VertexShader vertex(vertexSource);
    FragmentShader fragment(fragmentSource);

    AttachShader(vertex);
    AttachShader(fragment);
    Link();
}

Program::Program() : mProgram(glCreateProgram())
{
}

Program::~Program()
{
    if (mProgram)
    {
        if (CurrentProgram == mProgram)
        {
            CurrentProgram = 0;
        }

        glDeleteProgram(mProgram);
    }
}

Program::Program(Program&& other)
{
    mProgram = other.mProgram;
    mMVP.mLocation = other.mMVP.mLocation;

    other.mProgram = 0;
    other.mMVP.mLocation = GL_INVALID_VALUE;
}

Program & Program::operator=(Program&& other)
{
    mProgram = other.mProgram;
    mMVP.mLocation = other.mMVP.mLocation;

    other.mProgram = 0;
    other.mMVP.mLocation = GL_INVALID_VALUE;

    return *this;
}

Program & Program::AttachShader(const Shader& shader)
{
    glAttachShader(mProgram, shader.mShader);
    return *this;
}

Program & Program::AttachFeedback(const std::vector<const GLchar*> & varyings)
{
    glTransformFeedbackVaryings(mProgram, varyings.size(), varyings.data(), GL_INTERLEAVED_ATTRIBS);
    return *this;
}

Program & Program::Link()
{
    glBindAttribLocation(mProgram, Shader::Position, Shader::PositionName);
    glBindAttribLocation(mProgram, Shader::TexCoords, Shader::TexCoordsName);

    glLinkProgram(mProgram);

    GLint status;
    glGetProgramiv(mProgram, GL_LINK_STATUS, &status);
    if(!status)
    {
        GLsizei length;
        glGetProgramiv(mProgram, GL_INFO_LOG_LENGTH, &length);
        std::vector<GLchar> src(length);

        glGetProgramInfoLog(mProgram, length, NULL, src.data());
        throw std::runtime_error("Error linking program: " + std::string(src.data()));
    }

    mMVP.SetLocation(*this, "u_Projection");

    return *this;
}

Program& Program::Use()
{
    if (CurrentProgram != mProgram)
    {
        CurrentProgram = mProgram;
        glUseProgram(mProgram);
    }

    return *this;
}

void Program::Unuse()
{
    CurrentProgram = 0;
    glUseProgram(0);
}

Program& Program::SetMVP(const glm::mat4& mvp)
{
    mMVP.Set(mvp);
    return *this;
}

}}
