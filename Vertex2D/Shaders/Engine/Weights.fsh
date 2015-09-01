#version 150

precision highp float;

in vec2 v_texCoord;
in vec2 v_texCoordxp;
in vec2 v_texCoordxn;
in vec2 v_texCoordyp;
in vec2 v_texCoordyn;

uniform sampler2D u_texture; // this is the obstacles

out vec4 out_color;

void main()
{
    float o = texture(u_texture, v_texCoord).x;

    vec4 p;
    p.x = texture(u_texture, v_texCoordxp).x;
    p.y = texture(u_texture, v_texCoordxn).x;
    p.z = texture(u_texture, v_texCoordyp).x;
    p.w = texture(u_texture, v_texCoordyn).x;

    out_color = vec4(1.0) - p;
}
