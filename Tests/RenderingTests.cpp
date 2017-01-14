#include "gtest/gtest.h"
#include "Shapes.h"
#include "RenderTexture.h"
#include "Reader.h"
#include "Disable.h"

using namespace Vortex2D::Renderer;

void PrintData(int width, int height, const std::vector<float>& data)
{
    for (int j = 0; j < height; j++)
    {
        for (int i = 0; i < width; i++)
        {
            std::cout << "(" << data[i + j * width] << ")";
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;
}

void CheckTexture(int width, int height, const std::vector<float>& data, RenderTexture& texture)
{
    Reader reader(texture);
    reader.Read();

    for (int i = 0; i < width; i++)
    {
        for (int j = 0; j < height; j++)
        {
            float value = data[i + j * width];
            EXPECT_FLOAT_EQ(value, reader.GetFloat(i, j)) << "Value not equal at " << i << ", " << j;
        }
    }
}

void DrawSquare(int width, int height, std::vector<float>& data, const glm::vec2& centre, const glm::vec2& size)
{
    for (int i = 0; i < size.x; i++)
    {
        for (int j = 0; j < size.y; j++)
        {
            int x = i + centre.x;
            int y = j + centre.y;
            data[x + y * width] = 1.0f;
        }
    }
}

void DrawCircle(int width, int height, std::vector<float>& data, const glm::vec2& centre, float radius)
{
    for (int i = 0; i < width; i++)
    {
        for (int j = 0; j < height; j++)
        {
            glm::vec2 pos(i, j);
            pos = pos - centre + glm::vec2(1.0f);
            if (dot(pos, pos) <= (radius * radius))
            {
                data[i + j * width] = 1.0f;
            }
        }
    }

}

void DrawEllipse(int width, int height, std::vector<float>& data, const glm::vec2& centre, const glm::vec2& radius)
{
    for (int i = 0; i < width; i++)
    {
        for (int j = 0; j < height; j++)
        {
            glm::vec2 pos(i, j);
            pos = pos - centre + glm::vec2(1.0f);
            if (glm::dot(pos / radius, pos / radius) <= 1.0f)
            {
                data[i + j * width] = 1.0f;
            }
        }
    }
}

TEST(RenderingTest, RenderTexture)
{
    Disable d(GL_BLEND);

    RenderTexture texture(50, 50, Texture::PixelFormat::RF);

    std::vector<float> data(50*50, 3.5f);
    texture.Clear(glm::vec4(3.5f));

    CheckTexture(50, 50, data, texture);
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
    DrawSquare(50, 50, data, rect.Position, size);

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
