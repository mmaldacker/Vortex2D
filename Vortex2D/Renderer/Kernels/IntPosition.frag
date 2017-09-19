#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 1) uniform UBO
{
  ivec4 colour;
} u;

layout(location = 0) out ivec4 out_colour;

void main()
{
    out_colour = u.colour;
}
