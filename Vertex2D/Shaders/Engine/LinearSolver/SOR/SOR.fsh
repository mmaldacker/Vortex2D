#version 150

precision highp float;

in vec2 v_texCoord;

uniform sampler2D u_texture; // this is the pressure 
uniform sampler2D u_weights;
uniform float w;

const vec4 q = vec4(1.0);

out vec4 out_color;

void main()
{
    // cell.x is pressure and cell.y is div
    vec2 cell = texture(u_texture, v_texCoord).xy;

    vec4 p;
    p.x = textureOffset(u_texture, v_texCoord, ivec2(1,0)).x;
    p.y = textureOffset(u_texture, v_texCoord, ivec2(-1,0)).x;
    p.z = textureOffset(u_texture, v_texCoord, ivec2(0,1)).x;
    p.w = textureOffset(u_texture, v_texCoord, ivec2(0,-1)).x;


    vec4 c = texture(u_weights, v_texCoord);

    float pressure = mix(cell.x, (dot(p,c) + cell.y) / dot(q,c), w);

    out_color = vec4(pressure, cell.y, 0.0, 0.0);
}