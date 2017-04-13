//
//  Advection.cpp
//  Vortex
//

#include "Advection.h"

#include <Vortex2D/Renderer/Disable.h>


namespace Vortex2D { namespace Fluid {

namespace
{

const char* CommonFrag = GLSL(
    vec4 cubic(vec4 f1, vec4 f2, vec4 f3, vec4 f4, float xd)
    {
       float xd2 = xd * xd;
       float xd3 = xd2 * xd;

       return f1*(     - 0.5*xd  +     xd2 - 0.5*xd3) +
       f2*( 1.0           - 2.5*xd2 + 1.5*xd3) +
       f3*(       0.5*xd  + 2.0*xd2 - 1.5*xd3) +
       f4*(               - 0.5*xd2 + 0.5*xd3);
    }

    vec4 bicubic(vec4 t[16], vec2 f)
    {

       vec4 x = cubic(
                      cubic(t[0], t[4], t[8], t[12], f.y),
                      cubic(t[1], t[5], t[9], t[13], f.y),
                      cubic(t[2], t[6], t[10], t[14], f.y),
                      cubic(t[3], t[7], t[11], t[15], f.y),
                      f.x
                      );

       vec4 maxValue = max(max(t[5], t[6]), max(t[9], t[10]));
       vec4 minValue = min(min(t[5], t[6]), min(t[9], t[10]));

       return clamp(x, minValue, maxValue);
    }

    vec2 get_velocity(ivec2 pos, sampler2D velocity)
    {
       vec2 uv = texelFetch(velocity, pos, 0).xy;
       float up = texelFetch(velocity, pos + ivec2(1,0), 0).x;
       float vp = texelFetch(velocity, pos + ivec2(0,1), 0).y;

       return vec2(uv.x + up, uv.y + vp) * 0.5;
    }

    vec4[16] get_velocity_samples(ivec2 ij, sampler2D velocity)
    {
       vec4 t[16];
       for(int j = 0 ; j < 4 ; ++j)
       {
           for(int i = 0 ; i < 4 ; ++i)
           {
               t[i + 4*j] = vec4(get_velocity(ij + ivec2(i,j), velocity), 0.0, 0.0);
           }
       }

       return t;
    }

    vec4[16] get_samples(ivec2 ij, sampler2D texture)
    {
       vec4 t[16];
       for(int j = 0 ; j < 4 ; ++j)
       {
           for(int i = 0 ; i < 4 ; ++i)
           {
               t[i + 4*j] = texelFetch(texture, ij + ivec2(i,j), 0);
           }
       }
       return t;
    }

    vec2 interpolate_velocity(vec2 xy, sampler2D velocity)
    {
       ivec2 ij = ivec2(floor(xy)) - 1;
       vec2 f = xy - (ij + 1);

       return bicubic(get_velocity_samples(ij, velocity), f).xy;
    }

    vec4 interpolate(vec2 xy, sampler2D texture)
    {
       ivec2 ij = ivec2(floor(xy)) - 1;
       vec2 f = xy - (ij + 1);

       return bicubic(get_samples(ij, texture), f);
    }

);

const char * AdvectVelocityFrag = GLSL(
    uniform sampler2D u_velocity;
    uniform float delta;

    in vec2 v_texCoord;
    out vec4 out_color;

    const float a = 2.0/9.0;
    const float b = 3.0/9.0;
    const float c = 4.0/9.0;

    vec4 interpolate(vec2 xy, sampler2D texture);

    void main(void)
    {
        vec2 pos = gl_FragCoord.xy - 0.5;

        vec2 k1 = texture(u_velocity, v_texCoord).xy;
        vec2 k2 = interpolate(pos - 0.5*delta*k1, u_velocity).xy;
        vec2 k3 = interpolate(pos - 0.75*delta*k2, u_velocity).xy;

        out_color = interpolate(pos - a*delta*k1 - b*delta*k2 - c*delta*k3, u_velocity);
    }
);

const char * AdvectFrag = GLSL(
    uniform sampler2D u_texture;
    uniform sampler2D u_velocity;
    uniform float delta;

    in vec2 v_texCoord;
    out vec4 out_color;

    const float a = 2.0/9.0;
    const float b = 3.0/9.0;
    const float c = 4.0/9.0;

    vec4 interpolate(vec2 xy, sampler2D texture);
    vec2 interpolate_velocity(vec2 xy, sampler2D velocity);
    vec2 get_velocity(ivec2 pos, sampler2D velocity);

    void main(void)
    {
       vec2 pos = gl_FragCoord.xy - 0.5;

       vec2 k1 = get_velocity(ivec2(pos), u_velocity);
       vec2 k2 = interpolate_velocity(pos - 0.5*delta*k1, u_velocity);
       vec2 k3 = interpolate_velocity(pos - 0.75*delta*k2, u_velocity);

       out_color = interpolate(pos - a*delta*k1 - b*delta*k2 - c*delta*k3, u_texture);
    }
);

}

using Renderer::Back;

Advection::Advection(float dt, Renderer::Buffer& velocity)
    : mVelocity(velocity)
    , mVelocityAdvect(Renderer::Shader::TexturePositionVert, AdvectVelocityFrag, CommonFrag)
    , mAdvect(Renderer::Shader::TexturePositionVert, AdvectFrag, CommonFrag)
{
    mVelocityAdvect.Use().Set("delta", dt).Set("u_velocity", 0);
    mAdvect.Use().Set("delta", dt).Set("u_texture", 0).Set("u_velocity", 1);
}

void Advection::Advect()
{
    mVelocity.Swap();
    mVelocity = mVelocityAdvect(Back(mVelocity));
}

void Advection::Advect(Renderer::Buffer& buffer)
{
    Renderer::Disable d(GL_BLEND);
    buffer.Swap();
    buffer = mAdvect(Back(buffer), mVelocity);
}

}}
