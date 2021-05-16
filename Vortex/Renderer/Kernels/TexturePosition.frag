#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec2 v_texCoord;
layout(binding = 1) uniform sampler2D u_texture;

layout(binding = 2) uniform UBO
{
  vec4 colour;
} u;

layout(location = 0) out vec4 out_color;

void main()
{
    out_color = u.colour * texture(u_texture, v_texCoord);
}
