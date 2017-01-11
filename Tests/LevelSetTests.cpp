//
//  LevelSetSet.h
//  Vortex2D
//

#include "LevelSet.h"
#include "Shapes.h"
#include "Disable.h"
#include <iostream>

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

void PrintLevelSet(const int size)
{
    for (int i = 0; i < size; i++)
    {
        for (int j = 0; j < size; j++)
        {
            glm::vec2 pos{(float)i/size,(float)j/size};
            std::cout << "(" << size * boundary_phi_simple(pos) << ")";
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;
}

void PrintDifference(const Vortex2D::Renderer::Reader& reader, const int size)
{
    for (int i = 0; i < size; i++)
    {
        for (int j = 0; j < size; j++)
        {
            glm::vec2 pos{(float)i/size,(float)j/size};
            float value = size * boundary_phi_simple(pos);
            float diff = std::abs(reader.GetFloat(i, j) - value);

            if (diff > std::sqrt(2.0))
            {
                std::cout << "ERROR" << std::endl;
            }

            std::cout << "(" << diff << ")";
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;
}

void LevelSetTest(const float size)
{
    //PrintLevelSet(size);

    Vortex2D::Renderer::Disable e(GL_BLEND);

    Vortex2D::Fluid::LevelSet levelSet(glm::vec2{size});

    Vortex2D::Renderer::Ellipse circle(glm::vec2{rad0});
    circle.Colour = glm::vec4(1.0);
    circle.Position = glm::vec2(c0[0], c0[1]);
    circle.Scale = glm::vec2(size);

    levelSet.Clear(glm::vec4(-1.0));
    levelSet.Render(circle);
    //levelSet.Redistance(1000);
    auto reader = levelSet.Get();
    reader.Read().Print();

    //PrintDifference(reader, size);
}
