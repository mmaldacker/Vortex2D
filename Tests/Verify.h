//
//  Verify.h
//  Vortex2D
//

#include <gtest/gtest.h>

#include <Vortex2D/Renderer/Texture.h>
#include <Vortex2D/Renderer/Buffer.h>
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
void CheckTexture(const std::vector<T>& data, Vortex2D::Renderer::Texture& texture)
{
    std::vector<T> pixels(data.size());
    texture.CopyTo(pixels);

    for (int i = 0; i < texture.GetWidth(); i++)
    {
        for (int j = 0; j < texture.GetHeight(); j++)
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

/*
static void PrintVelocity(const glm::vec2& size, FluidSim& sim)
{
    for (std::size_t j = 0; j < size.y; j++)
    {
        for (std::size_t i = 0; i < size.x; i++)
        {
            std::cout << "(" << sim.u(i, j) << "," << sim.v(i, j) << ")";
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;
}
*/

/*
static void CheckData(int i, int j, float value, Reader& reader)
{
    EXPECT_FLOAT_EQ(value, reader.GetFloat(i, j)) << "Value not equal at " << i << ", " << j;
}

static void CheckData(int i, int j, const glm::vec4& value, Reader& reader)
{
    EXPECT_FLOAT_EQ(value.x, reader.GetVec4(i, j).x) << "Value not equal at " << i << ", " << j;
    EXPECT_FLOAT_EQ(value.y, reader.GetVec4(i, j).y) << "Value not equal at " << i << ", " << j;
    EXPECT_FLOAT_EQ(value.z, reader.GetVec4(i, j).z) << "Value not equal at " << i << ", " << j;
    EXPECT_FLOAT_EQ(value.w, reader.GetVec4(i, j).w) << "Value not equal at " << i << ", " << j;
}

static void CheckVelocity(Buffer& buffer, FluidSim& sim, float error = 1e-6)
{
    Reader reader(buffer);
    reader.Read();

    // FIXME need to check the entire velocity buffer
    for (std::size_t i = 1; i < buffer.Width() - 1; i++)
    {
        for (std::size_t j = 1; j < buffer.Height() - 1; j++)
        {
            EXPECT_NEAR(sim.u(i, j), reader.GetVec2(i, j).x, error) << "Mismatch at " << i << "," << j;
            EXPECT_NEAR(sim.v(i, j), reader.GetVec2(i, j).y, error) << "Mismatch at " << i << "," << j;
        }
    }
}
*/
