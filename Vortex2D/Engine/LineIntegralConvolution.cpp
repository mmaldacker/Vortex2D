//
//  LineIntegralConvolution.cpp
//  Vortex2D
//

#include "LineIntegralConvolution.h"

#include <random>

namespace Vortex2D { namespace Fluid {

namespace
{

const char * LICFrag = GLSL(
    uniform sampler2D u_velocity;
    uniform sampler2D u_noise;

    in vec2 v_texCoord;
    out vec4 out_color;

    void advection(float kernelLen, out float t_accum, out float w_accum)
    {
        int advections = 0;
        int max_advections = int(3.0 * kernelLen);
        float curLen = 0.0;
        float prevLen = 0.0;

        vec2  uv = texture(u_velocity, v_texCoord).xy;

        vec2 clp0 = gl_FragCoord.xy;
        vec2 clp1;

        while (curLen < kernelLen && advections < max_advections)
        {
            if (uv.x == 0.0 && uv.y == 0.0)
            {
                break;
            }
        }
    }

    void main()
    {
        float t_accum[2];
        float w_accum[2];

        advection(10.0, t_accum[0], w_accum[0]);
        advection(10.0, t_accum[1], w_accum[1]);

        out_color = vec4(vec3((t_accum[0] + t_accum[1]) / (w_accum[0] + w_accum[1])), 1.0);
    }
);

}

LineIntegralConvolution::LineIntegralConvolution(const glm::vec2 & size) : mLic(Renderer::Shader::TexturePositionVert, LICFrag), mOutput(size, 4)
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 255);

    std::size_t index = 3 * size.x * size.y;
    uint8_t * pixels = new uint8_t[index];
    int width = size.x;

    for (int j = 0; j < size.y; j++)
    {
        for (int i = 0; i < size.x; i++)
        {
            auto colour = dis(gen);
            pixels[3 * (j * width + i)] = colour;
            pixels[3 * (j * width + i) + 1] = colour;
            pixels[3 * (j * width + i) + 2] = colour;
        }
    }

    //mWhiteNoise = Renderer::Texture(size.x, size.y, Renderer::Texture::PixelFormat::RGB888, pixels);

    delete[] pixels;
}

LineIntegralConvolution::~LineIntegralConvolution()
{

}

void LineIntegralConvolution::Calculate(Renderer::Buffer& velocity)
{
    mOutput = mLic(velocity, mWhiteNoise);
}

void LineIntegralConvolution::Render(Renderer::RenderTarget& target, const glm::mat4& transform)
{
}

}}
