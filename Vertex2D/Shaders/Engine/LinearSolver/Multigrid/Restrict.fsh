#version 150

precision highp float;

in vec2 v_texCoord;
in vec2 v_texCoordx;
in vec2 v_texCoordy;
in vec2 v_texCoordxy;

uniform sampler2D u_texture;

out vec4 colour_out;

const vec4 q = vec4(1.0);

void main()
{
    vec4 p;
    p.x = texture(u_texture, v_texCoord).y;
    p.y = texture(u_texture, v_texCoordx).y;
    p.z = texture(u_texture, v_texCoordy).y;
    p.w = texture(u_texture, v_texCoordxy).y;

    colour_out = vec4(dot(p,q) * 0.25, 0.0, 0.0, 0.0);
}