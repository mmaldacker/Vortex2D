//
//  SuccessiveOverRelaxation.cpp
//  Vortex2D
//

#include "SuccessiveOverRelaxation.h"
#include "Common.h"
#include "Disable.h"

namespace Vortex2D { namespace Fluid {

namespace
{

const char * SorFrag = GLSL(
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
        if(mod(gl_FragCoord.x + gl_FragCoord.y,2.0) == 1.0)
        {
            discard;
        }
    }
);

}

using Renderer::Back;

SuccessiveOverRelaxation::SuccessiveOverRelaxation(const glm::vec2& size, int iterations)
    : mIterations(iterations)
    , mSor(Renderer::Shader::TexturePositionVert, SorFrag)
    , mStencil(Renderer::Shader::TexturePositionVert, CheckerMask)
    , mIdentity(Renderer::Shader::TexturePositionVert, Renderer::Shader::TexturePositionFrag)
{
    float w = 2.0f/(1.0f+std::sin(4.0f*std::atan(1.0f)/std::sqrt(size.x*size.y)));

    mSor.Use().Set("u_texture", 0).Set("u_weights", 1).Set("u_diagonals", 2).Set("w", w).Unuse();
}

SuccessiveOverRelaxation::SuccessiveOverRelaxation(const glm::vec2& size, int iterations, float w)
    : SuccessiveOverRelaxation(size, iterations)
{
    mSor.Use().Set("w", w).Unuse();
}

void SuccessiveOverRelaxation::Init(LinearSolver::Data& data)
{
    RenderMask(data.Pressure, data);

    Renderer::Enable e(GL_STENCIL_TEST);
    Renderer::DisableColorMask c;

    glStencilFunc(GL_NOTEQUAL, 1, 0xFF); // write value in stencil buffer
    glStencilOp(GL_KEEP, GL_KEEP, GL_INVERT); // invert value
    glStencilMask(0x02); // write in second place

    data.Pressure = mStencil();
    data.Pressure.Swap();
    data.Pressure = mStencil();
    data.Pressure.Swap();

    glStencilMask(0x00); // disable stencil writing
}

void SuccessiveOverRelaxation::Solve(LinearSolver::Data& data)
{
    for (int i  = 0; i < mIterations; ++i)
    {
        Step(data, true);
        Step(data, false);
    }
}

void SuccessiveOverRelaxation::Step(LinearSolver::Data& data, bool isRed)
{
    Renderer::Enable e(GL_STENCIL_TEST);
    glStencilMask(0x00);

    data.Pressure.Swap();

    glStencilFunc(GL_EQUAL, isRed ? 2 : 0, 0xFF);
    data.Pressure = mSor(Back(data.Pressure), data.Weights, data.Diagonal);

    glStencilFunc(GL_EQUAL, isRed ? 0 : 2, 0xFF);
    data.Pressure = mIdentity(Back(data.Pressure), data.Weights, data.Diagonal);
}

}}
