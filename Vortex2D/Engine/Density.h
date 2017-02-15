//
//  Density.h
//  Vortex2D
//

#ifndef Vortex2D_Density_h
#define Vortex2D_Density_h

#include <Vortex2D/Renderer/Operator.h>
#include <Vortex2D/Renderer/Drawable.h>
#include <Vortex2D/Renderer/Transformable.h>

#include <Vortex2D/Engine/Size.h>

namespace Vortex2D { namespace Fluid {

class World;

/**
 * @brief Used to move and render colour with the velocity field of the Engine
 */
class Density : public Renderer::Drawable, public Renderer::Transformable
{
public:
    Density(Dimensions dimensions);
    virtual ~Density();

    /**
     * @brief Renders some colours on the density grid
     */
    void Render(Renderer::Drawable & object);

    /**
     * @brief Advect the colours rendered with Render using the velocity field in Engine
     */
    void Advect(World & engine);

    /**
     * @brief Renders the colours to a RenderTarget
     */
    void Render(Renderer::RenderTarget & target, const glm::mat4 & transform) override;

private:
    Dimensions mDimensions;
    Renderer::Buffer mDensity;
    Renderer::Program mProgram;
};

}}

#endif /* defined(__Vortex2D__Density__) */
