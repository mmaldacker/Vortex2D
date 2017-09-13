#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec2 v_texCoord;
layout(binding = 1) uniform sampler2D u_texture;

layout(location = 0) out vec4 out_color;

void main()
{
    //out_color = texture(u_texture, v_texCoord);

    float value = texture(u_texture, v_texCoord).x;
    float alpha = 1.0 - clamp(4.0 * value + 0.5, 0.0, 1.0);

    out_color = vec4(0.0, 0.9, 0.5, alpha);
}
