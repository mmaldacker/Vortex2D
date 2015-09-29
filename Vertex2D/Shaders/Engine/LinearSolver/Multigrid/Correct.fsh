#version 150

precision highp float;

in vec2 v_texCoord;

uniform sampler2D u_texture;
uniform sampler2D u_residual;

out vec4 colour_out;

void main()
{
    vec2 p = texture(u_texture, v_texCoord).xy;
    float x = texture(u_residual, v_texCoord).x;

    colour_out = vec4(p.x+x, p.y, 0.0, 0.0);
}