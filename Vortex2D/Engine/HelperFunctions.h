//
//  Operator.h
//  Vortex2D
//

#ifndef Vortex2D_HelperFunctions_h
#define Vortex2D_HelperFunctions_h

#include <Vortex2D/Renderer/Common.h>

namespace Vortex2D { namespace Fluid {

static const char* WeightHelperFrag = GLSL(
    float fraction_inside(float a, float b)
    {
        if(a < 0.0 && b < 0.0)
            return 1.0;
        if(a < 0.0 && b >= 0.0)
            return a / (a - b);
        if(a >= 0.0 && b < 0.0)
            return b / (b - a);
        return 0.0;
    }

    // GLSL doesn't allow constant expressions as function arguments
    // so need to defined three versions for the three possible offsets:
    // (0,0) (2,0) (0,2)
    // FIXME indices are wrong (fix in FluidSim too)

    vec2 get_weight(vec2 texCoord, sampler2D solid_phi)
    {
        vec2 weight;
        weight.x = 1.0 - fraction_inside(textureOffset(solid_phi, texCoord, ivec2(0,2)).x,
                                         textureOffset(solid_phi, texCoord, ivec2(0,0)).x);
        weight.y = 1.0 - fraction_inside(textureOffset(solid_phi, texCoord, ivec2(2,0)).x,
                                         textureOffset(solid_phi, texCoord, ivec2(0,0)).x);

        return clamp(weight, vec2(0.0), vec2(1.0));
    }

    vec2 get_weightxp(vec2 texCoord, sampler2D solid_phi)
    {
        vec2 weight;
        weight.x = 1.0 - fraction_inside(textureOffset(solid_phi, texCoord, ivec2(2,2)).x,
                                         textureOffset(solid_phi, texCoord, ivec2(2,0)).x);
        weight.y = 1.0 - fraction_inside(textureOffset(solid_phi, texCoord, ivec2(4,0)).x,
                                         textureOffset(solid_phi, texCoord, ivec2(2,0)).x);

        return clamp(weight, vec2(0.0), vec2(1.0));
    }

    vec2 get_weightyp(vec2 texCoord, sampler2D solid_phi)
    {
        vec2 weight;
        weight.x = 1.0 - fraction_inside(textureOffset(solid_phi, texCoord, ivec2(0,4)).x,
                                         textureOffset(solid_phi, texCoord, ivec2(0,2)).x);
        weight.y = 1.0 - fraction_inside(textureOffset(solid_phi, texCoord, ivec2(2,2)).x,
                                         textureOffset(solid_phi, texCoord, ivec2(0,2)).x);

        return clamp(weight, vec2(0.0), vec2(1.0));
    }
);

}}

#endif
