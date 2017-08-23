#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) out vec4 out_colour;
layout(location = 1) flat in vec4 a_Centre;

layout(std140, binding = 1) uniform Size
{
    vec2 radius;
    mat2 rotation;
} s;

layout(binding = 2) uniform UBO
{
    vec4 colour;
} u;

void main()
{
    /*
    vec2 pos = s.rotation * a_Centre.xy;
    float distance = dot(pos / s.radius, pos / s.radius);
    if (distance - 1.0 <= 0.0)
    {
        out_colour = u.colour;
    }
    else
    {
        discard;
    }
    */

    //out_colour = vec4(s.radius, 0.0, 0.0);
    out_colour = a_Centre;
}
