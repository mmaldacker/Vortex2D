#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(push_constant) uniform Consts
{
  float radius;
}consts;

layout(binding = 1) uniform UBO
{
  mat4 mv;
}u;

layout(location = 0) out vec4 out_colour;

const float max_dist = 100000.0;

void main(void)
{
    vec2 pos = gl_FragCoord.xy;

    float value = -max_dist;
    vec2 p = (u.mv * vec4(0.0, 0.0, 0.0, 1.0)).xy;
    vec2 dir = p - pos;
    float dist = length(dir) - consts.radius;

    out_colour = vec4(dist);
}
