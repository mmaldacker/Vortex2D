//
//  RenderingTests.cpp
//  Vortex2D
//

#include "Helpers.h"

#include <Vortex2D/Renderer/Shapes.h>
#include <Vortex2D/Renderer/Disable.h>

#include <glm/gtx/rotate_vector.hpp>

using namespace Vortex2D::Renderer;

void DrawEllipse(int width, int height, std::vector<float>& data, const glm::vec2& centre, const glm::vec2& radius, float rotation = 0.0f)
{
    for (int i = 0; i < width; i++)
    {
        for (int j = 0; j < height; j++)
        {
            glm::vec2 pos(i, j);
            pos = glm::rotate(pos - centre + glm::vec2(1.0f), glm::radians(rotation));
            if (glm::dot(pos / radius, pos / radius) <= 1.0f)
            {
                data[i + j * width] = 1.0f;
            }
        }
    }
}

void DrawCircle(int width, int height, std::vector<float>& data, const glm::vec2& centre, float radius)
{
    DrawEllipse(width, height, data, centre, glm::vec2(radius));
}

TEST(RenderingTest, RenderTexture)
{
    Disable d(GL_BLEND);

    RenderTexture texture(50, 50, Texture::PixelFormat::RF);

    std::vector<float> data(50*50, 3.5f);
    texture.Clear(glm::vec4(3.5f));

    CheckTexture(50, 50, data, texture);
}

TEST(RenderingTest, WriteTexture)
{
    Disable d(GL_BLEND);

    RenderTexture texture(50, 50, Texture::PixelFormat::RF);

    std::vector<float> data(50*50, 0.0f);
    DrawSquare(50, 50, data, glm::vec2(10.0f, 15.0f), glm::vec2(5.0f, 8.0f), 1.0f);

    Writer writer(texture);
    writer.Write(data);

    CheckTexture(50, 50, data, texture);
}

TEST(RenderingTest, WriteVector2)
{
    Disable d(GL_BLEND);

    RenderTexture texture(50, 50, Texture::PixelFormat::RGF);

    std::vector<glm::vec2> data(50*50, glm::vec2(3.0f, -2.0f));
    data[20 + 50 * 2].x = 8.0f;
    data[12 + 50 * 4].y = 12.0f;

    Writer writer(texture);
    writer.Write(data);

    Reader reader(texture);
    reader.Read();

    for (int i = 0; i < 50; i++)
    {
        for (int j = 0; j < 50; j++)
        {
            glm::vec2 value = data[i + j * 50];
            EXPECT_FLOAT_EQ(value.x, reader.GetVec2(i, j).x) << "Value not equal at " << i << ", " << j;
            EXPECT_FLOAT_EQ(value.y, reader.GetVec2(i, j).y) << "Value not equal at " << i << ", " << j;
        }
    }
}

TEST(RenderingTest, WriteVector4)
{
    Disable d(GL_BLEND);

    RenderTexture texture(50, 50, Texture::PixelFormat::RGBAF);

    std::vector<glm::vec4> data(50*50, glm::vec4(3.0f, -2.0f, 0.5f, 4.6));
    data[20 + 50 * 2].x = 8.0f;
    data[12 + 50 * 4].y = 12.0f;
    data[43 + 50 * 12].z = -2.0f;
    data[32 + 50 * 38].w = -4.5f;

    Writer writer(texture);
    writer.Write(data);

    Reader reader(texture);
    reader.Read();

    for (int i = 0; i < 50; i++)
    {
        for (int j = 0; j < 50; j++)
        {
            glm::vec4 value = data[i + j * 50];
            EXPECT_FLOAT_EQ(value.x, reader.GetVec4(i, j).x) << "Value not equal at " << i << ", " << j;
            EXPECT_FLOAT_EQ(value.y, reader.GetVec4(i, j).y) << "Value not equal at " << i << ", " << j;
            EXPECT_FLOAT_EQ(value.z, reader.GetVec4(i, j).z) << "Value not equal at " << i << ", " << j;
            EXPECT_FLOAT_EQ(value.w, reader.GetVec4(i, j).w) << "Value not equal at " << i << ", " << j;
        }
    }
}

TEST(RenderingTest, Square)
{
    Disable d(GL_BLEND);

    glm::vec2 size = {10.0f, 20.0f};

    Rectangle rect(size);
    rect.Position = glm::vec2(5.0f, 7.0f);
    rect.Scale = glm::vec2(1.5f, 1.0f);
    rect.Colour = glm::vec4(1.0f);

    RenderTexture texture(50, 50, Texture::PixelFormat::RF);

    texture.Clear(glm::vec4(0.0));
    texture.Render(rect);

    size *= (glm::vec2)rect.Scale;
    std::vector<float> data(50*50, 0.0f);
    DrawSquare(50, 50, data, rect.Position, size, 1.0f);

    CheckTexture(50, 50, data, texture);
}

TEST(RenderingTest, Circle)
{
    Disable d(GL_BLEND);

    Ellipse ellipse(glm::vec2(5.0f));
    ellipse.Position = glm::vec2(10.0f, 15.0f);
    ellipse.Colour = glm::vec4(1.0f);

    RenderTexture texture(50, 50, Texture::PixelFormat::RF);

    texture.Clear(glm::vec4(0.0));
    texture.Render(ellipse);

    std::vector<float> data(50*50, 0.0f);
    DrawCircle(50, 50, data, ellipse.Position, 5.0f);

    CheckTexture(50, 50, data, texture);
}

TEST(RenderingTest, Ellipse)
{
    Disable d(GL_BLEND);

    glm::vec2 radius(4.0f, 7.0f);

    Ellipse ellipse(radius);
    ellipse.Position = glm::vec2(20.0f, 15.0f);
    ellipse.Colour = glm::vec4(1.0f);

    RenderTexture texture(50, 50, Texture::PixelFormat::RF);

    texture.Clear(glm::vec4(0.0));
    texture.Render(ellipse);

    std::vector<float> data(50*50, 0.0f);
    DrawEllipse(50, 50, data, ellipse.Position, radius);

    CheckTexture(50, 50, data, texture);
}

TEST(RenderingTest, ScaledEllipse)
{
    Disable d(GL_BLEND);

    glm::vec2 pos(20.0f, 15.0f);
    glm::vec2 radius(4.0f, 7.0f);

    Ellipse ellipse(radius);
    ellipse.Position = pos;
    ellipse.Colour = glm::vec4(1.0f);
    ellipse.Scale = glm::vec2(1.0f, 2.0f);

    RenderTexture texture(50, 50, Texture::PixelFormat::RF);

    texture.Clear(glm::vec4(0.0));
    texture.Render(ellipse);

    radius *= (glm::vec2)ellipse.Scale;
    std::vector<float> data(50*50, 0.0f);
    DrawEllipse(50, 50, data, ellipse.Position, radius);

    CheckTexture(50, 50, data, texture);
}

TEST(RenderingTest, RotatedEllipse)
{
    Disable d(GL_BLEND);

    glm::vec2 radius(4.0f, 7.0f);

    Ellipse ellipse(radius);
    ellipse.Position = glm::vec2(20.0f, 15.0f);
    ellipse.Colour = glm::vec4(1.0f);
    ellipse.Rotation = 33.0f;

    RenderTexture texture(50, 50, Texture::PixelFormat::RF);

    texture.Clear(glm::vec4(0.0));
    texture.Render(ellipse);

    std::vector<float> data(50*50, 0.0f);
    DrawEllipse(50, 50, data, ellipse.Position, radius, ellipse.Rotation);

    CheckTexture(50, 50, data, texture);
}

TEST(RenderingTest, RenderScaledEllipse)
{
    Disable d(GL_BLEND);

    glm::vec2 pos(10.0f, 10.0f);
    glm::vec2 radius(5.0f, 8.0f);

    Ellipse ellipse(radius);
    ellipse.Position = pos;
    ellipse.Colour = glm::vec4(1.0f);

    RenderTexture texture(50, 50, Texture::PixelFormat::RF);

    texture.Clear(glm::vec4(0.0));
    texture.Render(ellipse, glm::scale(glm::vec3(2.0f, 2.0f, 1.0f)));

    radius *= glm::vec2(2.0f);
    pos *= glm::vec2(2.0f);
    std::vector<float> data(50*50, 0.0f);
    DrawEllipse(50, 50, data, pos, radius);

    CheckTexture(50, 50, data, texture);
}
