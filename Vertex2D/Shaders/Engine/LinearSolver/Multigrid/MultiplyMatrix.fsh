#version 150

precision highp float;

in vec2 v_texCoord;
in vec2 v_texCoordxp;
in vec2 v_texCoordxn;
in vec2 v_texCoordyp;
in vec2 v_texCoordyn;

uniform sampler2D u_texture;
uniform sampler2D u_weights;

out vec4 colour_out;

const vec4 q = vec4(1.0);

void main()
{
    float x = texture(u_texture, v_texCoord).x;

    vec4 p;
    p.x = texture(u_texture, v_texCoordxp).x;
    p.y = texture(u_texture, v_texCoordxn).x;
    p.z = texture(u_texture, v_texCoordyp).x;
    p.w = texture(u_texture, v_texCoordyn).x;

    vec4 c = texture(u_weights, v_texCoord);

    float multiply = dot(q,c) * x - dot(p,c);
    colour_out = vec4(multiply, 0.0, 0.0, 0.0);
}