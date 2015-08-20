//
//  Emitter.h
//  Vortex
//
//  Created by Maximilian Maldacker on 28/04/2014.
//
//

#ifndef __Vortex__Emitter__
#define __Vortex__Emitter__

#include "FluidInput.h"
#include "Shapes.h"

namespace Fluid
{

class Emitter : public FluidInput
{
public:
    Emitter() = default;
    Emitter(float size);

    void SetSize(float size);

    float Magnitude;
    glm::vec4 Density;

    void RenderVelocity(const glm::mat4 & ortho) override;
    void RenderDensity(const glm::mat4 & ortho) override;
    
private:
    Renderer::Rectangle mShape;
};

}

#endif /* defined(__Vortex__Emitter__) */
