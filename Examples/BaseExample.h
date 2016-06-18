//
//  BaseExample.h
//  Vortex2D
//

#ifndef Vortex2D_BaseExample_h
#define Vortex2D_BaseExample_h

#include "Size.h"
#include "Engine.h"
#include "ConjugateGradient.h"
#include "Disable.h"

class BaseExample
{
public:
    BaseExample(Vortex2D::Fluid::Dimensions dimensions, float dt)
        : dimensions(dimensions)
        , solver(dimensions.Size)
        , engine(dimensions, solver, dt)
    {
    }

    virtual void Frame() = 0;
    virtual void Render(Vortex2D::Renderer::RenderTarget & target) = 0;

protected:
    Vortex2D::Fluid::Dimensions dimensions;
    Vortex2D::Fluid::ConjugateGradient solver;
    Vortex2D::Fluid::Engine engine;

    glm::vec4 green = glm::vec4(35.0f, 163.0f, 143.0f, 255.0f)/glm::vec4(255.0f);
    glm::vec4 gray = glm::vec4(182.0f,172.0f,164.0f, 255.0f)/glm::vec4(255.0f);
    glm::vec4 blue = glm::vec4(99.0f, 155.0f, 188.0f, 255.0f)/glm::vec4(255.0f);

};

#endif
