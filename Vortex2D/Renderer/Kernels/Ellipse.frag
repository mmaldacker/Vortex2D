#version 450
#extension GL_ARB_separate_shader_objects : enable


layout(location = 0) out vec4 out_colour;

layout(binding = 0) uniform UBO
{
  vec4 colour;
} u;

layout(binding = 1) uniform Size
{
    float size;
    vec2 radius;
    mat2 rotation;
} s;


void main()
{
    float size = 2 * s.size + 1;
    vec2 pos = (gl_PointCoord * size) - s.size;
    pos = s.rotation * pos;
    float distance = dot(pos / s.radius, pos / s.radius);
    if (distance - 1.0 <= 0.0)
    {
        out_colour = u.colour;
    }
    else
    {
        discard;
    }
}
