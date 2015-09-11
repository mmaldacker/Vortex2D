#version 150

precision highp float;

in vec2 v_texCoord;
in vec2 v_texCoordxp;
in vec2 v_texCoordxn;
in vec2 v_texCoordyp;
in vec2 v_texCoordyn;

uniform sampler2D u_texture;

out vec4 colour_out;

void main()
{
    float x = texture(u_texture, v_texCoord).x;

    colour_out = vec4(x, 0.0, 0.0, 0.0);
}