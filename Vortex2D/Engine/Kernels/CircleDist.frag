#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(push_constant) uniform Consts
{
  int width;
  int height;
  float radius;
}consts;

layout(binding = 0) uniform MVP
{
  mat4 value;
}mvp;

layout(std430, binding = 2) buffer Circle
{
  vec2 point;
}circle;

layout(location = 0) out vec4 out_colour;

void main(void)
{
    vec2 pos = gl_FragCoord.xy;

    float value = -max(consts.width, consts.height);
    vec2 dir = circle.point - pos;
    float scale = sqrt(mvp.value[0][0] * mvp.value[1][1]);
    float dist = length(dir) - consts.radius * scale;

    out_colour = vec4(value);
}
