//
//  Verify.h
//  Vortex2D
//

#include <gtest/gtest.h>

#include <Vortex2D/Renderer/Texture.h>
#include <Vortex2D/Renderer/Buffer.h>
#include <Vortex2D/Engine/LinearSolver/LinearSolver.h>
#include <vector>

template<typename T>
void PrintData(int width, int height, const std::vector<T>& data)
{
    for (int j = 0; j < height; j++)
    {
        for (int i = 0; i < width; i++)
        {
            T value = data[i + j * width];
            std::cout << "(" << value << ")";
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;
}

template<typename T>
void PrintTexture(Vortex2D::Renderer::Texture& texture)
{
    std::vector<T> pixels(texture.GetWidth() * texture.GetHeight());
    texture.CopyTo(pixels);

    for (uint32_t j = 0; j < texture.GetHeight(); j++)
    {
        for (uint32_t i = 0; i < texture.GetWidth(); i++)
        {
            T value = pixels[i + j * texture.GetWidth()];
            std::cout << "(" << value << ")";
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;
}

template<typename T>
void PrintBuffer(const glm::ivec2& size, Vortex2D::Renderer::Buffer& buffer)
{
    std::vector<T> pixels(size.x * size.y);
    buffer.CopyTo(pixels);

    for (int j = 0; j < size.x; j++)
    {
        for (int i = 0; i < size.y; i++)
        {
            T value = pixels[i + j * size.x];
            std::cout << "(" << value << ")";
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;
}

static void PrintWeights(const glm::ivec2& size, Vortex2D::Renderer::Buffer& buffer)
{
    std::vector<glm::vec2> pixels(size.x * size.y);
    buffer.CopyTo(pixels);

    for (int j = 0; j < size.x; j++)
    {
        for (int i = 0; i < size.y; i++)
        {
            glm::vec2 value = pixels[i + j * size.x];
            std::cout << "("
                      << value.x << ","
                      << value.y << ")";
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;
}

template<typename T>
void CheckTexture(const std::vector<T>& data, Vortex2D::Renderer::Texture& texture)
{
    std::vector<T> pixels(data.size());
    texture.CopyTo(pixels);

    for (uint32_t i = 0; i < texture.GetWidth(); i++)
    {
        for (uint32_t j = 0; j < texture.GetHeight(); j++)
        {
            T expectedValue = data[i + j * texture.GetWidth()];
            T value = pixels[i + j * texture.GetWidth()];
            EXPECT_FLOAT_EQ(expectedValue, value) << "Value not equal at " << i << ", " << j;
        }
    }
}

template<typename T>
void CheckBuffer(const std::vector<T>& data, Vortex2D::Renderer::Buffer& buffer)
{
    std::vector<T> pixels(data.size(), 0.0f);
    buffer.CopyTo(pixels);

    for (int i = 0; i < data.size(); i++)
    {
        T expectedValue = data[i];
        T value = pixels[i];
        EXPECT_FLOAT_EQ(expectedValue, value) << "Value not equal at " << i;
    }
}
