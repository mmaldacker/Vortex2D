//
//  LinearSolver.cpp
//  Vertex2D
//

#include "LinearSolver.h"

namespace Vortex2D { namespace Fluid {

namespace
{

const char* MaskFrag = GLSL(
    in vec2 v_texCoord;
    out vec4 out_color;

    uniform sampler2D u_texture;

    void main()
    {
        float x = texture(u_texture, v_texCoord).x;

        if (x == 0.0)
        {
            discard;
        }
    }
);

}

using Renderer::Back;

LinearSolver::Data::Data(const glm::vec2& size)
    : Weights(size, 4)
    , Diagonal(size, 1)
    , Pressure(size, 2, true, true)
{

}

LinearSolver::Parameters::Parameters(unsigned iterations, float errorTolerance)
    : Iterations(iterations)
    , ErrorTolerance(errorTolerance)
    , OutIterations(0)
    , OutError(0.0f)
{

}

bool LinearSolver::Parameters::IsFinished(unsigned iterations, float error) const
{
    if (Iterations > 0)
    {
        return iterations > Iterations  || error < ErrorTolerance;
    }
    else
    {
        return error < ErrorTolerance;
    }
}


LinearSolver::LinearSolver()
    : mMask(Renderer::Shader::TexturePositionVert, MaskFrag)
{

}

void LinearSolver::RenderMask(Renderer::Buffer& destination, Data& data)
{
    destination.ClearStencil();
}


}}
