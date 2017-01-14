//
//  LevelSetSet.h
//  Vortex2D
//

#include "gtest/gtest.h"
#include "LevelSet.h"
#include "Reader.h"
#include "Shapes.h"
#include "Disable.h"

using namespace Vortex2D::Renderer;
using namespace Vortex2D::Fluid;

const glm::vec2 c0(0.5,0.5), c1(0.7,0.5), c2(0.3,0.35), c3(0.5,0.7);
const float rad0 = 0.4,  rad1 = 0.1,  rad2 = 0.1,   rad3 = 0.1;

float circle_phi(const glm::vec2& position, const glm::vec2& centre, float radius)
{
   return glm::distance(position,centre) - radius;
}

float boundary_phi_simple(const glm::vec2& position)
{
   return -circle_phi(position, c0, rad0);
}

float boundary_phi_complex(const glm::vec2& position)
{
   float phi0 = -circle_phi(position, c0, rad0);
   float phi1 = circle_phi(position, c1, rad1);
   float phi2 = circle_phi(position, c2, rad2);
   float phi3 = circle_phi(position, c3, rad3);

   return glm::min(glm::min(phi0,phi1),glm::min(phi2,phi3));
}

void PrintLevelSet(int size, float (*phi)(const glm::vec2&))
{
    for (int i = 0; i < size; i++)
    {
        for (int j = 0; j < size; j++)
        {
            glm::vec2 pos(i,j);
            pos += glm::vec2(1.0f);
            pos /= glm::vec2(size);
            std::cout << "(" << size * phi(pos) << ")";
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;
}

void CheckDifference(int size, Reader& reader, float (*phi)(const glm::vec2&))
{
    for (int i = 0; i < size; i++)
    {
        for (int j = 0; j < size; j++)
        {
            glm::vec2 pos(i,j);
            pos += glm::vec2(1.0f);
            pos /= glm::vec2(size);
            float value = size * phi(pos);
            float diff = std::abs(reader.GetFloat(i, j) - value);

            EXPECT_LT(diff, std::sqrt(2.0f));
        }
    }
}

TEST(LevelSetTests, SimpleCircle)
{
    Vortex2D::Renderer::Disable e(GL_BLEND);

    glm::vec2 size(50.0f);

    Vortex2D::Fluid::LevelSet levelSet(size);

    Vortex2D::Renderer::Ellipse circle(glm::vec2{rad0} * size);
    circle.Colour = glm::vec4(1.0f);
    circle.Position = glm::vec2(c0[0], c0[1]) * size;


    levelSet.Clear(glm::vec4(-1.0f));
    levelSet.Render(circle);
    levelSet.Redistance(500);

    CheckDifference(size.x, levelSet.Get().Read(), boundary_phi_simple);
}

TEST(LevelSetTests, ComplexCircles)
{
    Vortex2D::Renderer::Disable e(GL_BLEND);

    glm::vec2 size(50.0f);

    Vortex2D::Fluid::LevelSet levelSet(size);

    Vortex2D::Renderer::Ellipse circle0(glm::vec2{rad0} * size);
    Vortex2D::Renderer::Ellipse circle1(glm::vec2{rad1} * size);
    Vortex2D::Renderer::Ellipse circle2(glm::vec2{rad2} * size);
    Vortex2D::Renderer::Ellipse circle3(glm::vec2{rad3} * size);

    circle0.Colour = glm::vec4(1.0);
    circle1.Colour = circle2.Colour = circle3.Colour = glm::vec4(-1.0f);

    circle0.Position = glm::vec2(c0[0], c0[1]) * size;
    circle1.Position = glm::vec2(c1[0], c1[1]) * size;
    circle2.Position = glm::vec2(c2[0], c2[1]) * size;
    circle3.Position = glm::vec2(c3[0], c3[1]) * size;

    levelSet.Clear(glm::vec4(-1.0));
    levelSet.Render(circle0);
    levelSet.Render(circle1);
    levelSet.Render(circle2);
    levelSet.Render(circle3);
    levelSet.Redistance(500);

    CheckDifference(size.x, levelSet.Get().Read(), boundary_phi_complex);
}



