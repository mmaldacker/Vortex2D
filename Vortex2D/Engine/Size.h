//
//  Size.h
//  Vortex2D
//

#ifndef Vortex2D_Size_h
#define Vortex2D_Size_h

#include <Vortex2D/Renderer/Common.h>

namespace Vortex2D { namespace Fluid {

/**
 * @brief Represents the size of the fluid grid. This is useful if we want a different
 * resolution or scale than one grid cell per pixel.
 */
struct Dimensions
{
    /**
     * @brief The grid size will be: size/scale
     * @param size the size at which we will draw boundaries, forces, etc
     * @param scale the scale of the actual grid
     */
    Dimensions(const glm::vec2 & size, const float scale)
        : Scale(scale)
        , Size(glm::floor(size/scale))
        , RealSize(size)
        , InvScale(glm::scale(glm::vec3(1.0f/scale, 1.0f/scale, 1.0f)))
    {
    }

    const float Scale;
    const glm::vec2 Size;
    const glm::vec2 RealSize;
    const glm::mat4 InvScale;
};

}}

#endif
