//
//  GaussSeidel.cpp
//  Vortex2D
//

#include "GaussSeidel.h"

#include <glm/gtc/constants.hpp>

namespace Vortex2D { namespace Fluid {

namespace
{

const char * GaussSeidelFrag = GLSL(
    in vec2 v_texCoord;
    out vec4 out_color;

    uniform sampler2D u_texture; // this is the pressure
    uniform sampler2D u_weights;
    uniform sampler2D u_diagonals;
    uniform float w;

    void main()
    {
        // cell.x is pressure and cell.y is div
        vec2 cell = texture(u_texture, v_texCoord).xy;

        vec4 p;
        p.x = textureOffset(u_texture, v_texCoord, ivec2(1,0)).x;
        p.y = textureOffset(u_texture, v_texCoord, ivec2(-1,0)).x;
        p.z = textureOffset(u_texture, v_texCoord, ivec2(0,1)).x;
        p.w = textureOffset(u_texture, v_texCoord, ivec2(0,-1)).x;

        vec4 c = texture(u_weights, v_texCoord);
        float d = texture(u_diagonals, v_texCoord).x;

        float pressure = mix(cell.x, (cell.y - dot(p, c)) / d, w);

        out_color = vec4(pressure, cell.y, 0.0, 0.0);
    }
);

const char * CheckerMask = GLSL(
    void main()
    {
        if (mod(gl_FragCoord.x + gl_FragCoord.y, 2.0) == 1.0)
        {
            discard;
        }
    }
);

}

using Renderer::Back;

GaussSeidel::GaussSeidel()
    : mGaussSeidel(Renderer::Shader::TexturePositionVert, GaussSeidelFrag)
    , mStencil(Renderer::Shader::TexturePositionVert, CheckerMask)
    , mIdentity(Renderer::Shader::TexturePositionVert, Renderer::Shader::TexturePositionFrag)
{
    mGaussSeidel.Use().Set("u_texture", 0).Set("u_weights", 1).Set("u_diagonals", 2);
}

GaussSeidel::GaussSeidel(const glm::vec2& size)
    : GaussSeidel()
{
    float w = 2.0f/(1.0f+std::sin(glm::pi<float>()/std::sqrt(size.x*size.y)));
    SetW(w);
}

void GaussSeidel::Build(Data&,
                        Renderer::Operator&,
                        Renderer::Operator&,
                        Renderer::Buffer&,
                        Renderer::Buffer&)
{
}

void GaussSeidel::Init(Data& data)
{
}

void GaussSeidel::Solve(Data& data, Parameters& params)
{
    // FIXME implement solving within error tolerance
    assert(params.Iterations > 0);

    for (unsigned i  = 0; !params.IsFinished(i, 0.0f); ++i)
    {
        Step(data, 0x02, 0x01);
        Step(data, 0x01, 0x02);

        params.OutIterations = i;
    }
}

void GaussSeidel::Step(Data& data, uint8_t redMask, uint8_t blackMask)
{

    data.Pressure.Swap();

    data.Pressure = mGaussSeidel(Back(data.Pressure), data.Weights, data.Diagonal);

    data.Pressure = mIdentity(Back(data.Pressure), data.Weights, data.Diagonal);
}

void GaussSeidel::SetW(float w)
{
    mGaussSeidel.Use();
}

}}
