//
//  Transfer.cpp
//  Vortex2D
//

#include "Transfer.h"

namespace Vortex2D { namespace Fluid {

namespace
{

const char* ProlongateFrag = GLSL(
    out vec4 colour_out;

    uniform sampler2D u_texture;

    void main()
    {
        ivec2 pos = ivec2(gl_FragCoord.xy * 0.5);
        vec2 m = mod(gl_FragCoord.xy - 0.5, vec2(2.0));

        ivec2 t = ivec2(m.x == 0.0 ? -1.0 : 1.0,
                        m.y == 0.0 ? -1.0 : 1.0);

        float p = 0.0;
        p += 9.0 * texelFetch(u_texture, pos, 0).x;
        p += 3.0 * texelFetch(u_texture, pos + ivec2(t.x, 0), 0).x;
        p += 3.0 * texelFetch(u_texture, pos + ivec2(0, t.y), 0).x;
        p += 1.0 * texelFetch(u_texture, pos + t, 0).x;

        colour_out = vec4(p / 16.0, 0.0, 0.0, 0.0);
    }
);

const char* RestrictFrag = GLSL(
    out vec4 colour_out;

    uniform sampler2D u_texture;

    float coefficients[16] = float[](1.0, 3.0, 3.0, 1.0,
                                     3.0, 9.0, 9.0, 3.0,
                                     3.0, 9.0, 9.0, 3.0,
                                     1.0, 3.0, 3.0, 1.0);

    void main()
    {
        ivec2 pos = 2 * ivec2(gl_FragCoord.xy - 0.5);

        float p = 0.0;

        for (int i = -1; i <= 2; i++)
        {
            for (int j = -1; j <= 2; j++)
            {
                int index = (i + 1) + 4 * (j + 1);
                p += coefficients[index] * texelFetch(u_texture, pos + ivec2(i, j), 0).x;
            }
        }

        colour_out = vec4(p / 64.0, 0.0, 0.0, 0.0);
    }
);

}

Transfer::Transfer()
    : prolongate(Renderer::Shader::PositionVert, ProlongateFrag)
    , restrict(Renderer::Shader::PositionVert, RestrictFrag)
{
    prolongate.Use().Set("u_texture", 0).Unuse();
    restrict.Use().Set("u_texture", 0).Unuse();
}

Renderer::OperatorContext Transfer::Prolongate(Renderer::Buffer& input)
{
    return prolongate(input);
}

Renderer::OperatorContext Transfer::Restrict(Renderer::Buffer& input)
{
    return restrict(input);
}

}}
