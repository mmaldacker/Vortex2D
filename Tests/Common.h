//
//  Common.h
//  Vortex2D
//

#include <vector>
#include <iostream>

#include "gtest/gtest.h"
#include "Disable.h"
#include "RenderTexture.h"
#include "Operator.h"
#include "Reader.h"

using namespace Vortex2D::Renderer;

static void PrintData(int width, int height, const std::vector<float>& data)
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

static void CheckTexture(int width, int height, const std::vector<float>& data, Reader& reader)
{
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

static void CheckTexture(int width, int height, const std::vector<float>& data, Buffer& buffer)
{
    Reader reader(buffer);
    CheckTexture(width, height, data, reader);
}

static void CheckTexture(int width, int height, const std::vector<float>& data, RenderTexture& texture)
{
    Reader reader(texture);
    CheckTexture(width, height, data, reader);
}
