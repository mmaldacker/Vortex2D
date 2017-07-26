//
//  LevelSetSet.h
//  Vortex2D
//

#include "VariationalHelpers.h"

#include <Vortex2D/Renderer/Shapes.h>
#include <Vortex2D/Engine/LevelSet.h>

using namespace Vortex2D::Renderer;
using namespace Vortex2D::Fluid;

extern Device* device;

void PrintLevelSet(int size, float (*phi)(const Vec2f&))
{
    for (int i = 0; i < size; i++)
    {
        for (int j = 0; j < size; j++)
        {
            Vec2f pos(i,j);
            pos += Vec2f(1.0f);
            pos /= (float)size;
            std::cout << "(" << size * phi(pos) << ")";
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;
}

void PrintLevelSet(Texture& texture)
{
    std::vector<float> pixels(texture.GetWidth() * texture.GetHeight());
    texture.CopyTo(pixels.data(), 4);

    for (int j = 0; j < texture.GetHeight() / 2; j++)
    {
        for (int i = 0; i < texture.GetWidth() / 2; i++)
        {
            float value = 0.0f;
            value += pixels[i * 2 + (j * 2) * texture.GetWidth()];
            value += pixels[i * 2 + 1 + (j * 2) * texture.GetWidth()];
            value += pixels[i * 2 + (j * 2 + 1) * texture.GetWidth()];
            value += pixels[i * 2 + 1 + (j * 2 + 1) * texture.GetWidth()];
            value *= 0.25f * 0.5f;

            std::cout << "(" << value << ")";
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;
}

void CheckDifference(Texture& texture, float (*phi)(const Vec2f&))
{
    std::vector<float> pixels(texture.GetWidth() * texture.GetHeight());
    texture.CopyTo(pixels.data(), 4);

    for (uint32_t j = 0; j < texture.GetHeight() / 2; j++)
    {
        for (uint32_t i = 0; i < texture.GetWidth() / 2; i++)
        {
            Vec2f pos(i,j);
            pos += Vec2f(1.0f);
            pos /= 0.5f * texture.GetWidth();
            float value = 0.5f * texture.GetWidth() * phi(pos);

            float readerValue = 0.0f;
            readerValue += pixels[i * 2 + (j * 2) * texture.GetWidth()];
            readerValue += pixels[i * 2 + 1 + (j * 2) * texture.GetWidth()];
            readerValue += pixels[i * 2 + (j * 2 + 1) * texture.GetWidth()];
            readerValue += pixels[i * 2 + 1 + (j * 2 + 1) * texture.GetWidth()];
            readerValue *= 0.25f * 0.5f;

            float diff = std::abs(std::abs(value) - std::abs(readerValue));

            EXPECT_LT(diff, 0.8f) << "Mismatch at " << i << ", " << j; // almost sqrt(0.5)
        }
    }
}

TEST(LevelSetTests, SimpleCircle)
{
    glm::vec2 size(20.0f);
    glm::vec2 doubleSize(glm::vec2(2.0f) * size);

    LevelSet levelSet(*device, doubleSize, 500);
    Texture outTexture(*device, doubleSize.x, doubleSize.y, vk::Format::eR32Sfloat, true);

    Ellipse circle(*device, glm::vec2{rad0} * doubleSize, glm::vec4(0.5f));
    circle.Position = glm::vec2(c0[0], c0[1]) * doubleSize;

    circle.Initialize({levelSet});
    circle.Update(levelSet.Orth, glm::mat4());

    levelSet.Record([&](vk::CommandBuffer commandBuffer)
    {
        Clear(doubleSize.x, doubleSize.y, glm::vec4(-0.5f)).Draw(commandBuffer);
        circle.Draw(commandBuffer, {levelSet});
    });

    levelSet.Submit();

    levelSet.Reinitialise();

    device->Handle().waitIdle();

    CommandBuffer cmd(*device);
    cmd.Record([&](vk::CommandBuffer commandBuffer)
    {
       outTexture.CopyFrom(commandBuffer, levelSet);
    });

    cmd.Submit();
    cmd.Wait();

    CheckDifference(outTexture, boundary_phi);
}

TEST(LevelSetTests, ComplexCircles)
{
    glm::vec2 size(50.0f);
    glm::vec2 doubleSize(glm::vec2(2.0f) * size);

    LevelSet levelSet(*device, doubleSize, 2000);
    Texture outTexture(*device, doubleSize.x, doubleSize.y, vk::Format::eR32Sfloat, true);

    Ellipse circle0(*device, glm::vec2{rad0} * doubleSize, glm::vec4(1.0f));
    Ellipse circle1(*device, glm::vec2{rad1} * doubleSize, glm::vec4(-1.0f));
    Ellipse circle2(*device, glm::vec2{rad2} * doubleSize, glm::vec4(-1.0f));
    Ellipse circle3(*device, glm::vec2{rad3} * doubleSize, glm::vec4(-1.0f));

    circle0.Position = glm::vec2(c0[0], c0[1]) * doubleSize;
    circle1.Position = glm::vec2(c1[0], c1[1]) * doubleSize;
    circle2.Position = glm::vec2(c2[0], c2[1]) * doubleSize;
    circle3.Position = glm::vec2(c3[0], c3[1]) * doubleSize;

    circle0.Initialize({levelSet});
    circle1.Initialize({levelSet});
    circle2.Initialize({levelSet});
    circle3.Initialize({levelSet});
    circle0.Update(levelSet.Orth, glm::mat4());
    circle1.Update(levelSet.Orth, glm::mat4());
    circle2.Update(levelSet.Orth, glm::mat4());
    circle3.Update(levelSet.Orth, glm::mat4());

    levelSet.Record([&](vk::CommandBuffer commandBuffer)
    {
        Clear(doubleSize.x, doubleSize.y, glm::vec4(-1.0f)).Draw(commandBuffer);
        circle0.Draw(commandBuffer, {levelSet});
        circle1.Draw(commandBuffer, {levelSet});
        circle2.Draw(commandBuffer, {levelSet});
        circle3.Draw(commandBuffer, {levelSet});
    });

    levelSet.Submit();
    levelSet.Reinitialise();

    device->Handle().waitIdle();

    CommandBuffer cmd(*device);
    cmd.Record([&](vk::CommandBuffer commandBuffer)
    {
        outTexture.CopyFrom(commandBuffer, levelSet);
    });

    cmd.Submit();
    cmd.Wait();

    CheckDifference(outTexture, complex_boundary_phi);
}

/*
TEST(LevelSetTests, Extrapolate)
{
    glm::vec2 size(50);

    Buffer solidPhi(glm::vec2(2)*size, 1);

    std::vector<float> solidData(4 * size.x * size.y, 1.0);
    DrawSquare(100, 100, solidData, glm::vec2(10.0f), glm::vec2(20.0f), -1.0f);
    Writer(solidPhi).Write(solidData);

    LevelSet liquidPhi(size);

    std::vector<float> liquidData(4 * size.x * size.y, 0.0);
    Writer(liquidPhi).Write(liquidData);

    liquidPhi.Extrapolate(solidPhi);

    Reader reader(liquidPhi);
    reader.Read();

    for (int i = 0; i < 9; i++)
    {
        EXPECT_FLOAT_EQ(-1.0f, reader.GetFloat(i + 5, 5));
        EXPECT_FLOAT_EQ(-1.0f, reader.GetFloat(5, i + 5));
        EXPECT_FLOAT_EQ(-1.0f, reader.GetFloat(i + 5, 10));
        EXPECT_FLOAT_EQ(-1.0f, reader.GetFloat(10, i + 5));
    }
}
*/
