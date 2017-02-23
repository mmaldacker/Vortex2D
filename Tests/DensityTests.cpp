//
//  DensityTests.cpp
//  Vortex2D
//

#include "Helpers.h"

#include <Vortex2D/Renderer/Disable.h>
#include <Vortex2D/Renderer/Shapes.h>

#include <Vortex2D/Engine/Density.h>

using namespace Vortex2D::Renderer;
using namespace Vortex2D::Fluid;

TEST(DensityTests, Render)
{
    Dimensions dimensions(glm::vec2(30.0f), 1.0f);
    Density density(dimensions);

    glm::vec2 size(10.0f, 5.0f);
    Rectangle area(size);
    area.Position = glm::vec2(4.0f);
    area.Colour = glm::vec4(1.0f);

    density.Render(area);

    RenderTexture texture(30, 30, Texture::PixelFormat::RGBA8888);
    texture.Render(density);

    std::vector<glm::vec4> data(30*30, glm::vec4(0.0f));
    DrawSquare(30, 30, data, area.Position, size, glm::vec4(1.0f));

    CheckTexture(data, texture);
}
