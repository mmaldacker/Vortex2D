//
//  ShapeDrawer.h
//  Vortex2D
//

#include <glm/glm.hpp>
#include <vector>

#include <glm/gtx/rotate_vector.hpp>

void DrawEllipse(int width,
                 int height,
                 std::vector<float>& data,
                 const glm::vec2& centre,
                 const glm::vec2& radius,
                 float rotation = 0.0f);

void DrawCircle(int width,
                int height,
                std::vector<float>& data,
                const glm::vec2& centre,
                float radius);

template<typename T>
void DrawSquare(int width, int height, std::vector<T>& data, const glm::vec2& centre, const glm::vec2& size, T value)
{
    for (int i = 0; i < size.x; i++)
    {
        for (int j = 0; j < size.y; j++)
        {
            int x = i + centre.x;
            int y = j + centre.y;
            data[x + y * width] = value;
        }
    }
}

