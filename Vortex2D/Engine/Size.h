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
    Dimensions(const glm::ivec2 & size, const float scale);

    const float Scale;
    const glm::ivec2 Size;
    const glm::mat4 InvScale;
};

}}

#endif
