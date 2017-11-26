//
//  Density.h
//  Vortex2D
//

#ifndef Vortex2D_Density_h
#define Vortex2D_Density_h

#include <Vortex2D/Renderer/Sprite.h>
#include <Vortex2D/Renderer/RenderTexture.h>

namespace Vortex2D { namespace Fluid {

class Density : public Renderer::RenderTexture, public Renderer::Sprite
{
public:
    Density(const Renderer::Device& device, const glm::ivec2& size, vk::Format format);

    friend  class Advection;
private:
    Renderer::Texture mFieldBack;
};


}}

#endif
