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
const char * Shader::ColourName = "a_Colour";

Shader::Shader(GLuint shader) : mShader(shader)
{
}

Shader::~Shader()
{
    if(mShader)
    {
        SDL_Log("Delete shader %d", mShader);
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
        SDL_Log("Error compiling shader %s", src);
		throw std::runtime_error("Error compiling shader: " + std::string(src));
	}

    return *this;
}

VertexShader::VertexShader() : Shader(glCreateShader(GL_VERTEX_SHADER))
{
}

FragmentShader::FragmentShader() : Shader(glCreateShader(GL_FRAGMENT_SHADER))
{
}

int Program::CurrentProgram = -1;

Program::Program() : mProgram(glCreateProgram())
{
}

Program::Program(const std::string & vertexSource, const std::string & fragmentSource) : Program()
{
    VertexShader vertex;
    vertex.Source(vertexSource).Compile();

    FragmentShader fragment;
    fragment.Source(fragmentSource).Compile();

    AttachShader(vertex);
    AttachShader(fragment);
    Link();
}

Program::~Program()
{
    if(mProgram)
    {
        SDL_Log("Delete program %d", mProgram);
        glDeleteProgram(mProgram);
    }
}

Program::Program(Program && other)
{
    *this = std::move(other);
}

Program & Program::operator=(Program && other)
{
    mProgram = other.mProgram;
    mMVP.mLocation = other.mMVP.mLocation;

    other.mProgram = 0;
    other.mMVP.mLocation = -1;

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
    glBindAttribLocation(mProgram, Shader::Colour, Shader::ColourName);

    glLinkProgram(mProgram);

    GLint status;
    glGetProgramiv(mProgram, GL_LINK_STATUS, &status);
    if(!status)
    {
		GLsizei length;
		glGetProgramiv(mProgram, GL_INFO_LOG_LENGTH, &length);
		GLchar src[length];

		glGetProgramInfoLog(mProgram, length, NULL, src);
        SDL_Log("Error linking program: %s", src);
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

Program & Program::SetMVP(const glm::mat4 &mvp)
{
    mMVP.Set(mvp);
    return *this;
}

Program Program::mTexturePositionProgram;
Program Program::mPositionProgram;
Program Program::mColourTexturePositionProgram;
Program Program::mColourPositionProgram;
Program Program::mCircleProgram;

Program & Program::TexturePositionProgram()
{
    if(!mTexturePositionProgram.mProgram)
    {
        Program program("TexturePosition.vsh", "TexturePosition.fsh");
        program.Use().Set("u_Texture", 0);
        mTexturePositionProgram = std::move(program);
    }

    return mTexturePositionProgram;
}

Program & Program::PositionProgram()
{
    if(!mPositionProgram.mProgram)
    {
        Program program("Position.vsh", "Position.fsh");
        mPositionProgram = std::move(program);
    }

    return mPositionProgram;
}

Program & Program::ColourTexturePositionProgram()
{
    if(!mColourTexturePositionProgram.mProgram)
    {
        Program program("ColourTexturePosition.vsh", "ColourTexturePosition.fsh");
        program.Use().Set("u_Texture", 0);
        mColourTexturePositionProgram = std::move(program);
    }

    return mColourTexturePositionProgram;
}

Program & Program::ColourPositionProgram()
{
    if(!mColourPositionProgram.mProgram)
    {
        Program program("ColourPosition.vsh", "ColourPosition.fsh");
        mColourPositionProgram = std::move(program);
    }

    return mColourPositionProgram;
}

Program & Program::CircleProgram()
{
    if(!mCircleProgram.mProgram)
    {
        Program program("Circle.vsh", "Circle.fsh");
        mCircleProgram = std::move(program);
    }

    return mCircleProgram;
}

}
