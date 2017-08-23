//
//  ShapeDrawer.cpp
//  Vortex2D
//

#include "ShapeDrawer.h"

void DrawEllipse(int width,
                 int height,
                 std::vector<float>& data,
                 const glm::vec2& centre,
                 const glm::vec2& radius,
                 float rotation)
{
    for (int i = 0; i < width; i++)
    {
        for (int j = 0; j < height; j++)
        {
            glm::vec2 pos(i + 0.5f, j + 0.5f);
            pos = glm::rotate(pos - centre, -glm::radians(rotation));
            if (glm::dot(pos / radius, pos / radius) <= 1.0f)
            {
                data[i + j * width] = 1.0f;
            }
        }
    }
}

void DrawCircle(int width,
                int height,
                std::vector<float>& data,
                const glm::vec2& centre,
                float radius)
{
    DrawEllipse(width, height, data, centre, glm::vec2(radius));
}
