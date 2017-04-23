//
//  ShaderTests.cpp
//  Vortex2D
//

#include "Helpers.h"

#include <Vortex2D/Renderer/Disable.h>

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

const char * MultiplySubFrag = GLSL(
    in vec2 v_texCoord;
    out vec4 colour_out;

    uniform sampler2D u_x;
    uniform sampler2D u_y;
    uniform sampler2D u_scalar;

    void main()
    {
        float x = texture(u_x, v_texCoord).x;
        float y = texture(u_y, v_texCoord).x;
        float alpha = texture(u_scalar, vec2(0.5)).x;

        colour_out = vec4(x - alpha * y, 0.0, 0.0, 0.0);
    }
);

const char * BorderFrag = GLSL(
    in vec2 v_texCoord;
    out vec4 colour_out;

    uniform sampler2D u_texture;

    void main()
    {
        vec4 p;
        p.x = textureOffset(u_texture, v_texCoord, ivec2(1,0)).x;
        p.y = textureOffset(u_texture, v_texCoord, ivec2(-1,0)).x;
        p.z = textureOffset(u_texture, v_texCoord, ivec2(0,1)).x;
        p.w = textureOffset(u_texture, v_texCoord, ivec2(0,-1)).x;

        const vec4 q = vec4(1.0);

        colour_out = vec4(dot(p, q), 0.0, 0.0, 0.0);
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
    CheckTexture(data, buffer);
}

TEST(ShaderTests, MultipleShaders)
{
    Disable d(GL_BLEND);

    Operator op(PositionVert, PositionWithCommonFrag, CommonFrag);
    op.Use().Set("u_Colour", glm::vec4(1.2f));

    Buffer buffer(glm::vec2(50, 50), 1);
    buffer = op();

    std::vector<float> data(50*50, 1.2f);
    CheckTexture(data, buffer);
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

    CheckTexture(data, buffer);
}

TEST(ShaderTests, OperatorBack)
{
    Disable d(GL_BLEND);

    Buffer x({1,1}, 1, true);
    Buffer y({1,1}, 1);
    Buffer z({1,1}, 1);
    Operator op(Shader::TexturePositionVert, MultiplySubFrag);
    op.Use().Use().Set("u_x", 0).Set("u_y", 1).Set("u_scalar", 2);

    float xValue = 1.0f;
    float yValue = 0.5f;
    float zValue = 3.0f;

    Writer(x).Write(&xValue);
    Writer(y).Write(&yValue);
    Writer(z).Write(&zValue);

    x.Swap();
    x = op(Back(x), y, z);

    Reader reader(x);
    reader.Read();

    ASSERT_FLOAT_EQ(xValue - yValue * zValue, reader.GetFloat(0, 0));
}

TEST(ShaderTests, BorderColour)
{
  Disable d(GL_BLEND);

  Buffer x({2,2}, 1);
  RenderTexture tex(2, 2, Texture::PixelFormat::RF);
  tex.Clear(glm::vec4(0.0f));
  tex.ClampToBorder();
  tex.BorderColour(glm::vec4(-1.0f));

  Operator op(Shader::TexturePositionVert, BorderFrag);
  op.Use().Set("u_texture", 0);

  x = op(tex);

  std::vector<float> data(4, -2.0f);

  CheckTexture(data, x);
}