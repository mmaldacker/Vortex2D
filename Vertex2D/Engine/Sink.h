//
//  Sink.h
//  Vortex
//
//  Created by Maximilian Maldacker on 08/05/2014.
//
//

#ifndef __Vortex__Sink__
#define __Vortex__Sink__

#include "FluidInput.h"
#include "Shapes.h"

namespace Fluid
{

class Sink : public FluidInput
{
public:
    Sink() = default;
    Sink(float size);

    void SetSize(float size);

    float Magnitude;
    
    void RenderObstacleVelocity(const glm::mat4 & ortho);

private:
    Renderer::Rectangle mShape;
};

}

#endif /* defined(__Vortex__Sink__) */
