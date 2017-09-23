#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec2 v_texCoord;
layout(binding = 1) uniform sampler2D u_texture;

layout(location = 0) out vec4 out_color;

layout(push_constant) uniform Consts
{
  vec4 colour;
  float scale;
}consts;

void main()
{
    float value = texture(u_texture, v_texCoord).x;
    float alpha = 1.0 - clamp(consts.scale * value + 0.5, 0.0, 1.0);

    out_color = vec4(consts.colour.rgb, 1.0 - alpha);
}
