//
//  LevelSetSet.h
//  Vortex2D
//

#include "gtest/gtest.h"
#include "LevelSet.h"
#include "Reader.h"
#include "Shapes.h"
#include "Disable.h"
#include "Helpers.h"

using namespace Vortex2D::Renderer;
using namespace Vortex2D::Fluid;

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

void PrintLevelSet(Array2f& array)
{
    for (int j = 0; j < array.nj; j++)
    {
        for (int i = 0; i < array.ni; i++)
        {
            std::cout << "(" << array(i, j) << ")";
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;
}

void PrintLevelSet(Buffer& buffer)
{
    Reader reader(buffer);
    reader.Read();

    for (int j = 0; j < buffer.Height() / 2; j++)
    {
        for (int i = 0; i < buffer.Width() / 2; i++)
        {
            float value = 0.0f;
            value += reader.GetFloat(i * 2, j * 2);
            value += reader.GetFloat(i * 2 + 1, j * 2);
            value += reader.GetFloat(i * 2, j * 2 + 1);
            value += reader.GetFloat(i * 2 + 1, j * 2 + 1);
            value *= 0.25f * 0.5f;

            std::cout << "(" << value << ")";
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;
}

void CheckDifference(int size, Buffer& buffer, float (*phi)(const Vec2f&))
{
    Reader reader(buffer);
    reader.Read();

    for (int j = 0; j < buffer.Height() / 2; j++)
    {
        for (int i = 0; i < buffer.Width() / 2; i++)
        {
            Vec2f pos(i,j);
            pos += Vec2f(1.0f);
            pos /= (float)size;
            float value = size * phi(pos);

            float readerValue = 0.0f;
            readerValue += reader.GetFloat(i * 2, j * 2);
            readerValue += reader.GetFloat(i * 2 + 1, j * 2);
            readerValue += reader.GetFloat(i * 2, j * 2 + 1);
            readerValue += reader.GetFloat(i * 2 + 1, j * 2 + 1);
            readerValue *= 0.25f * 0.5f;

            float diff = std::abs(value - readerValue);

            EXPECT_LT(diff, 0.8f); // almost sqrt(0.5)
        }
    }
}

void CheckLiquidPhi(Buffer& buffer, FluidSim& sim)
{
    Reader reader(buffer);
    reader.Read();

    for (int i = 0; i < buffer.Width(); i++)
    {
        for (int j = 0; j < buffer.Height(); j++)
        {
            EXPECT_FLOAT_EQ(sim.liquid_phi(i, j), reader.GetFloat(i, j));
        }
    }
}

TEST(LevelSetTests, SimpleCircle)
{
    Vortex2D::Renderer::Disable e(GL_BLEND);

    glm::vec2 size(20.0f);
    glm::vec2 doubleSize(glm::vec2(2.0f) * size);

    Vortex2D::Fluid::LevelSet levelSet(doubleSize);

    Vortex2D::Renderer::Ellipse circle(glm::vec2{rad0} * doubleSize);
    circle.Colour = glm::vec4(0.5f);
    circle.Position = glm::vec2(c0[0], c0[1]) * doubleSize;

    levelSet.Clear(glm::vec4(-0.5f));
    levelSet.Render(circle);
    levelSet.Redistance(500);

    CheckDifference(size.x, levelSet, boundary_phi);
}

TEST(LevelSetTests, ComplexCircles)
{
    Vortex2D::Renderer::Disable e(GL_BLEND);

    glm::vec2 size(50.0f);
    glm::vec2 doubleSize(glm::vec2(2.0f) * size);

    Vortex2D::Fluid::LevelSet levelSet(doubleSize);

    Vortex2D::Renderer::Ellipse circle0(glm::vec2{rad0} * doubleSize);
    Vortex2D::Renderer::Ellipse circle1(glm::vec2{rad1} * doubleSize);
    Vortex2D::Renderer::Ellipse circle2(glm::vec2{rad2} * doubleSize);
    Vortex2D::Renderer::Ellipse circle3(glm::vec2{rad3} * doubleSize);

    circle0.Colour = glm::vec4(1.0);
    circle1.Colour = circle2.Colour = circle3.Colour = glm::vec4(-1.0f);

    circle0.Position = glm::vec2(c0[0], c0[1]) * doubleSize;
    circle1.Position = glm::vec2(c1[0], c1[1]) * doubleSize;
    circle2.Position = glm::vec2(c2[0], c2[1]) * doubleSize;
    circle3.Position = glm::vec2(c3[0], c3[1]) * doubleSize;

    levelSet.Clear(glm::vec4(-1.0));
    levelSet.Render(circle0);
    levelSet.Render(circle1);
    levelSet.Render(circle2);
    levelSet.Render(circle3);
    levelSet.Redistance(500);

    CheckDifference(size.x, levelSet, complex_boundary_phi);
}

TEST(LevelSetTests, Extrapolate)
{
    Disable d(GL_BLEND);

    glm::vec2 size(50);

    FluidSim sim;
    sim.initialize(1.0f, size.x, size.y);
    sim.set_boundary(complex_boundary_phi);

    AddParticles(size, sim, complex_boundary_phi);

    sim.compute_phi();

    Buffer solidPhi(glm::vec2(2)*size, 1);
    solidPhi.ClampToEdge();
    SetSolidPhi(solidPhi, sim);

    LevelSet liquidPhi(size);
    SetLiquidPhi(liquidPhi, sim);

    sim.extrapolate_phi();

    liquidPhi.Extrapolate(solidPhi);

    CheckLiquidPhi(liquidPhi, sim);
}
