#version 150

precision highp float;

in vec2 v_texCoord;

uniform sampler2D u_texture;
uniform sampler2D u_diagonals;

out vec4 colour_out;

void main()
{
    float x = texture(u_texture, v_texCoord).x;
    float diag = texture(u_diagonals, v_texCoord).x;

    colour_out = vec4(x/diag, 0.0, 0.0, 0.0);
}
