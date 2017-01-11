#include "gtest/gtest.h"
#include "Shapes.h"
#include "RenderTexture.h"
#include "Reader.h"
#include "Disable.h"

using namespace Vortex2D::Renderer;


void PrintData(int width, int height, const std::vector<float>& data)
{
    for (int i = 0; i < width; i++)
    {
        for (int j = 0; j < height; j++)
        {
            std::cout << "(" << data[i + j * width] << ")";
        }
        std::cout << std::endl;
    }
}

void CheckTexture(int width, int height, const std::vector<float>& data, RenderTexture& texture)
{
    Reader reader(texture);
    reader.Read().Print();

    for (int i = 0; i < width; i++)
    {
        for (int j = 0; j < height; j++)
        {
            float value = data[i + j * width];
            EXPECT_FLOAT_EQ(value, reader.GetFloat(i, j)) << "Value not equal at " << i << ", " << j;
        }
    }
}

TEST(RenderingTest, Square)
{
    Disable d(GL_BLEND);

    Rectangle rect(glm::vec2(10.0f, 20.0f));
    rect.Position = glm::vec2(5.0f, 7.0f);
    rect.Scale = glm::vec2(1.5f, 1.0f);
    rect.Colour = glm::vec4(1.2f);

    RenderTexture texture(50, 50, Texture::PixelFormat::RF);

    texture.Clear(glm::vec4(0.0));
    texture.Render(rect);

    std::vector<float> data(50*50, 0.0f);

    for (int i = 0; i < 15; i++)
    {
        for (int j = 0; j < 20; j++)
        {
            int x = i + 5;
            int y = j + 7;
            data[x + y * 50] = 1.2f;
        }
    }


    CheckTexture(50, 50, data, texture);
}

TEST(RenderingTest, Circle)
{
    Disable d(GL_BLEND);

    Ellipse ellipse(glm::vec2(5.0f));
    ellipse.Position = glm::vec2(10.0f, 15.0f);
    ellipse.Colour = glm::vec4(3.0f);

    RenderTexture texture(50, 50, Texture::PixelFormat::RF);

    texture.Clear(glm::vec4(0.0));
    texture.Render(ellipse);

    std::vector<float> data(50*50, 0.0f);

    for (int i = 0; i < 50; i++)
    {
        for (int j = 0; j < 50; j++)
        {
            int x = i - 10;
            int y = j - 15;
            if ((x * x + y * y) <= (5.0f * 5.0f))
            {
                data[i + j * 50] = 3.0f;
            }
        }
    }

    PrintData(50, 50, data);
    std::cout << std::endl;
    CheckTexture(50, 50, data, texture);
}
