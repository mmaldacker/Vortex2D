#version 150

precision highp float;

in vec2 v_texCoord;

uniform sampler2D u_texture; // this is the obstacles

out vec4 out_color;

void main()
{
    vec4 p;
    //FIXME 2 needs to be a uniform
    p.x = textureOffset(u_texture, v_texCoord, ivec2(2,0)).x;
    p.y = textureOffset(u_texture, v_texCoord, ivec2(-2,0)).x;
    p.z = textureOffset(u_texture, v_texCoord, ivec2(0,2)).x;
    p.w = textureOffset(u_texture, v_texCoord, ivec2(0,-2)).x;

    out_color = vec4(1.0) - p;
}
