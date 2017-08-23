#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) out vec4 out_colour;
layout(location = 1) in vec4 a_Centre;

layout(std140, binding = 1) uniform Size
{
    vec2 centre;
    vec2 radius;
    mat2 rotation;
} s;

layout(binding = 2) uniform UBO
{
    vec4 colour;
} u;

void main()
{
    vec2 halfDim = s.centre;

    vec2 pos = gl_FragCoord.xy - (halfDim * a_Centre.xy + halfDim);
    pos = s.rotation * pos;
    float distance = dot(pos / s.radius, pos / s.radius);
    if (distance <= 1.0)
    {
        out_colour = u.colour;
    }
    else
    {
        discard;
    }
}
