#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform UniformBufferObject
{
  vec4 colour;
} u;

layout(location = 0) out vec4 out_colour;

void main()
{
    out_colour = u.colour;
}
