//
//  Shader.cpp
//  Vortex
//
//  Created by Maximilian Maldacker on 06/04/2014.
//
//

#include "Shader.h"
#include "ResourcePath.h"
#include <fstream>

namespace Renderer
{

const char * Shader::PositionName = "a_Position";
const char * Shader::TexCoordsName = "a_TexCoords";

Shader::Shader(GLuint shader) : mShader(shader)
{
}

Shader::~Shader()
{
    if(mShader)
    {
        glDeleteShader(mShader);
    }
}

Shader & Shader::Source(const std::string & source)
{
    std::ifstream sourceFile(getResourcePath() + source);
    std::string sourceFileContent((std::istreambuf_iterator<char>(sourceFile)), std::istreambuf_iterator<char>());

    auto c_str = sourceFileContent.c_str();
    glShaderSource(mShader, 1, &c_str, NULL);
    return *this;
}

Shader & Shader::Compile()
{
    glCompileShader(mShader);

    GLint status;
    glGetShaderiv(mShader, GL_COMPILE_STATUS, &status);

	if(!status)
    {
		GLsizei length;
		glGetShaderiv(mShader, GL_INFO_LOG_LENGTH, &length);
		GLchar src[length];

		glGetShaderInfoLog(mShader, length, NULL, src);
		throw std::runtime_error("Error compiling shader: \n" + std::string(src));
	}

    return *this;
}

VertexShader::VertexShader() : Shader(glCreateShader(GL_VERTEX_SHADER))
{
}

FragmentShader::FragmentShader() : Shader(glCreateShader(GL_FRAGMENT_SHADER))
{
}

int Program::CurrentProgram = 0;

Program::Program(const std::string & vertexSource, const std::string & fragmentSource) : mProgram(glCreateProgram())
{
    VertexShader vertex;
    vertex.Source(vertexSource).Compile();

    FragmentShader fragment;
    fragment.Source(fragmentSource).Compile();

    AttachShader(vertex);
    AttachShader(fragment);
    Link();
}

Program::Program() : mProgram(0)
{
}

Program::~Program()
{
    if(mProgram)
    {
        glDeleteProgram(mProgram);
    }
}

Program::Program(Program && other)
{
    mProgram = other.mProgram;
    mMVP.mLocation = other.mMVP.mLocation;

    other.mProgram = 0;
    other.mMVP.mLocation = GL_INVALID_VALUE;
}

Program & Program::operator=(Program && other)
{
    mProgram = other.mProgram;
    mMVP.mLocation = other.mMVP.mLocation;

    other.mProgram = 0;
    other.mMVP.mLocation = GL_INVALID_VALUE;

    return *this;
}

Program & Program::AttachShader(const Shader &shader)
{
    glAttachShader(mProgram, shader.mShader);
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
		GLchar src[length];

		glGetProgramInfoLog(mProgram, length, NULL, src);
		throw std::runtime_error("Error linking program: " + std::string(src));
    }

    mMVP.SetLocation(*this, "u_Projection");

    return *this;
}

Program & Program::Use()
{
    if(CurrentProgram != mProgram)
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

Program & Program::SetMVP(const glm::mat4 &mvp)
{
    mMVP.Set(mvp);
    return *this;
}

Program & Program::TexturePositionProgram()
{
    static Program program;
    if(!program.mProgram)
    {
        program = Program("TexturePosition.vsh", "TexturePosition.fsh");
        program.Use().Set("u_texture", 0).Unuse();
    }

    return program;
}

Program & Program::PositionProgram()
{
    static Program program;
    if(!program.mProgram)
    {
        program = Program("Position.vsh", "Position.fsh");
    }

    return program;
}

}
