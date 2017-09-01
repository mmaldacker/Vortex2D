//
//  LevelSetSet.h
//  Vortex2D
//

#include "VariationalHelpers.h"
#include "Verify.h"
#include "Renderer/ShapeDrawer.h"

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

void CheckDifference(Texture& texture, float (*phi)(const Vec2f&))
{
    std::vector<float> pixels(texture.GetWidth() * texture.GetHeight());
    texture.CopyTo(pixels);

    for (uint32_t j = 0; j < texture.GetHeight(); j++)
    {
        for (uint32_t i = 0; i < texture.GetWidth(); i++)
        {
            Vec2f pos(i,j);
            pos += Vec2f(1.0f);
            pos /= (float)texture.GetWidth();
            float value = texture.GetWidth() * phi(pos);

            float readerValue = pixels[i + j * texture.GetWidth()];
            float diff = std::abs(value - readerValue);

            EXPECT_LT(diff, 0.8f) << "Mismatch at " << i << ", " << j; // almost sqrt(0.5)
        }
    }
}

TEST(LevelSetTests, SimpleCircle)
{
    glm::ivec2 size(50);

    LevelSet levelSet(*device, size, 2000);
    Texture outTexture(*device, size.x, size.y, vk::Format::eR32Sfloat, true);

    Ellipse circle(*device, glm::vec2{rad0} * glm::vec2(size), glm::vec4(0.5f));
    circle.Position = glm::vec2(c0[0], c0[1]) * glm::vec2(size) - glm::vec2(0.5f);

    circle.Initialize({levelSet});
    circle.Update(levelSet.Orth, glm::mat4());

    levelSet.Record([&](vk::CommandBuffer commandBuffer)
    {
        Clear(size.x, size.y, glm::vec4(-0.5f)).Draw(commandBuffer);
        circle.Draw(commandBuffer, {levelSet});
    });

    levelSet.Submit();

    levelSet.Reinitialise();

    device->Handle().waitIdle();

    ExecuteCommand(*device, [&](vk::CommandBuffer commandBuffer)
    {
       outTexture.CopyFrom(commandBuffer, levelSet);
    });

    CheckDifference(outTexture, boundary_phi);
}

TEST(LevelSetTests, ComplexCircles)
{
    glm::ivec2 size(50);

    LevelSet levelSet(*device, size, 2000);
    Texture outTexture(*device, size.x, size.y, vk::Format::eR32Sfloat, true);

    Ellipse circle0(*device, glm::vec2{rad0} * glm::vec2(size), glm::vec4(1.0f));
    Ellipse circle1(*device, glm::vec2{rad1} * glm::vec2(size), glm::vec4(-1.0f));
    Ellipse circle2(*device, glm::vec2{rad2} * glm::vec2(size), glm::vec4(-1.0f));
    Ellipse circle3(*device, glm::vec2{rad3} * glm::vec2(size), glm::vec4(-1.0f));

    circle0.Position = glm::vec2(c0[0], c0[1]) * glm::vec2(size) - glm::vec2(0.5f);
    circle1.Position = glm::vec2(c1[0], c1[1]) * glm::vec2(size) - glm::vec2(0.5f);
    circle2.Position = glm::vec2(c2[0], c2[1]) * glm::vec2(size) - glm::vec2(0.5f);
    circle3.Position = glm::vec2(c3[0], c3[1]) * glm::vec2(size) - glm::vec2(0.5f);

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
        Clear(size.x, size.y, glm::vec4(-1.0f)).Draw(commandBuffer);
        circle0.Draw(commandBuffer, {levelSet});
        circle1.Draw(commandBuffer, {levelSet});
        circle2.Draw(commandBuffer, {levelSet});
        circle3.Draw(commandBuffer, {levelSet});
    });

    levelSet.Submit();
    levelSet.Reinitialise();

    device->Handle().waitIdle();

    ExecuteCommand(*device, [&](vk::CommandBuffer commandBuffer)
    {
        outTexture.CopyFrom(commandBuffer, levelSet);
    });

    CheckDifference(outTexture, complex_boundary_phi);
}

TEST(LevelSetTests, Extrapolate)
{
    glm::ivec2 size(50);

    Texture localSolidPhi(*device, size.x, size.y, vk::Format::eR32Sfloat, true);
    Texture localLiquidPhi(*device, size.x, size.y, vk::Format::eR32Sfloat, true);
    Texture solidPhi(*device, size.x, size.y, vk::Format::eR32Sfloat, false);

    std::vector<float> solidData(size.x * size.y, 1.0);
    DrawSquare(size.x, size.y, solidData, glm::vec2(10.0f), glm::vec2(20.0f), -1.0f);
    localSolidPhi.CopyFrom(solidData);

    ExecuteCommand(*device, [&](vk::CommandBuffer commandBuffer)
    {
        solidPhi.CopyFrom(commandBuffer, localSolidPhi);
    });

    LevelSet liquidPhi(*device, size);

    liquidPhi.ExtrapolateInit(solidPhi);

    liquidPhi.Extrapolate();

    device->Handle().waitIdle();

    ExecuteCommand(*device, [&](vk::CommandBuffer commandBuffer)
    {
        localLiquidPhi.CopyFrom(commandBuffer, liquidPhi);
    });

    std::vector<float> liquidData(size.x*size.y);
    localLiquidPhi.CopyTo(liquidData);

    for (int i = 0; i < 9; i++)
    {
        EXPECT_FLOAT_EQ(-0.5f, liquidData[i + 10 + 10 * size.x]);
        EXPECT_FLOAT_EQ(-0.5f, liquidData[10 + (i + 10) * size.x]);
        EXPECT_FLOAT_EQ(-0.5f, liquidData[i + 10 + 20 * size.x]);
        EXPECT_FLOAT_EQ(-0.5f, liquidData[20 + (i + 10) * size.x]);
    }
}
