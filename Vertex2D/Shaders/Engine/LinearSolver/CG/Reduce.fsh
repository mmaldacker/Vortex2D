#version 150

precision highp float;

in vec2 v_texCoord;

uniform sampler2D u_texture;

out vec4 colour_out;

const vec4 q = vec4(1.0);

void main()
{
    vec4 p;
    p.x = texture(u_texture, v_texCoord).x;
    p.y = textureOffset(u_texture, v_texCoord, ivec2(0,1)).x;
    p.z = textureOffset(u_texture, v_texCoord, ivec2(1,0)).x;
    p.w = textureOffset(u_texture, v_texCoord, ivec2(1,1)).x;

    colour_out = vec4(dot(p,q), 0.0, 0.0, 0.0);
}
