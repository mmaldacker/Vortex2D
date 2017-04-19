//
//  Size.cpp
//  Vortex2D
//

#include "Size.h"

#include <glm/gtx/transform.hpp>

namespace Vortex2D { namespace Fluid {

Dimensions::Dimensions(const glm::vec2 & size, const float scale)
    : Scale(scale)
    , Size(glm::floor(size/scale))
    , InvScale(glm::scale(glm::vec3(1.0f/scale, 1.0f/scale, 1.0f)))
{
}

}}
