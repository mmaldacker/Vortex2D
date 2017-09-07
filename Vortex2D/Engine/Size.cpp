//
//  Size.cpp
//  Vortex2D
//

#include "Size.h"

#include <glm/gtx/transform.hpp>

namespace Vortex2D { namespace Fluid {

Dimensions::Dimensions(const glm::ivec2 & size, const float scale)
    : Scale(scale)
    , Size(glm::floor(glm::vec2(size)/scale))
    , InvScale(glm::scale(glm::vec3(1.0f/scale, 1.0f/scale, 1.0f)))
{
}

}}
