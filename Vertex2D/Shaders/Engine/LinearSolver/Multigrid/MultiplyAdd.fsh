#version 150

precision highp float;

in vec2 v_texCoord;

uniform sampler2D u_texture;
uniform sampler2D u_other;
uniform sampler2D u_scalar;

out vec4 colour_out;

void main()
{
    float x = texture(u_texture, v_texCoord).x;
    float y = texture(u_other, v_texCoord).x;
    float alpha = texture(u_scalar, vec2(0.5));

    colour_out = vec4(x+alpha*y, 0.0, 0.0, 0.0);
}