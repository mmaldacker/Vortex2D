//
//  ShaderTests.cpp
//  Vortex2D
//

#include "gtest/gtest.h"
#include "Common.h"
#include "Disable.h"
#include "Operator.h"
#include "Buffer.h"
#include "Writer.h"

using namespace Vortex2D::Renderer;

namespace
{

const char * PositionVert = GLSL(
    in vec2 a_Position;
    uniform mat4 u_Projection;

    void main()
    {
        gl_Position = u_Projection * vec4(a_Position, 0.0, 1.0);
    }
);

const char * PositionFrag = GLSL(
    out vec4 out_color;
    uniform vec4 u_Colour;

    void main()
    {
        out_color = u_Colour;
    }
);

const char * PositionWithCommonFrag = GLSL(
    out vec4 out_color;

    vec4 GetColour();

    void main()
    {
        out_color = GetColour();
    }
);

const char * CommonFrag = GLSL(
    uniform vec4 u_Colour;

    vec4 GetColour()
    {
        return u_Colour;
    }
);

}

TEST(ShaderTests, SingleShader)
{
    Disable d(GL_BLEND);

    Operator op(PositionVert, PositionFrag);
    op.Use().Set("u_Colour", glm::vec4(1.2f));

    Buffer buffer(glm::vec2(50, 50), 1);
    buffer = op();

    std::vector<float> data(50*50, 1.2f);
    CheckTexture(50, 50, data, buffer);
}

TEST(ShaderTests, MultipleShaders)
{
    Disable d(GL_BLEND);

    Operator op(PositionVert, PositionWithCommonFrag, CommonFrag);
    op.Use().Set("u_Colour", glm::vec4(1.2f));

    Buffer buffer(glm::vec2(50, 50), 1);
    buffer = op();

    std::vector<float> data(50*50, 1.2f);
    CheckTexture(50, 50, data, buffer);
}

TEST(ShaderTests, OperatorBind)
{
    Disable d(GL_BLEND);

    Texture texture(50, 50, Texture::PixelFormat::RF);
    Operator op(Shader::TexturePositionVert, Shader::TexturePositionFrag);
    op.Use().Set("u_texture", 0);

    std::vector<float> data(50*50, 1.2f);
    Writer(texture).Write(data);

    Buffer buffer(glm::vec2(50, 50), 1);
    buffer = op(texture);

    CheckTexture(50, 50, data, buffer);
}
