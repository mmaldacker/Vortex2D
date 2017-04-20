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

Shader::Shader(Type shader, const char* source)
{
}

Shader::~Shader()
{
}

VertexShader::VertexShader(const char* source)
    : Shader(Type::Vertex, source)
{
}

FragmentShader::FragmentShader(const char* source)
    : Shader(Type::Fragment, source)
{
}

Program::Program(const char* vertexSource, const char* fragmentSource)
{
    VertexShader vertex(vertexSource);
    FragmentShader fragment(fragmentSource);

    AttachShader(vertex);
    AttachShader(fragment);
    Link();
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
