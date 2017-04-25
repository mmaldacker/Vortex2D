//
//  RenderTarget.cpp
//  Vortex2D
//

#include "RenderTarget.h"

#include <glm/gtc/matrix_transform.hpp>

namespace Vortex2D { namespace Renderer {

RenderTarget::RenderTarget(float width, float height)
    : Orth(glm::ortho(0.0f, width, 0.0f, height))
    , Width(width)
    , Height(height)
{

}

}}
