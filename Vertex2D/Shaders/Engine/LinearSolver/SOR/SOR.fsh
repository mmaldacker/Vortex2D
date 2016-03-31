#version 150

precision highp float;

in vec2 v_texCoord;

uniform sampler2D u_texture; // this is the pressure 
uniform sampler2D u_weights;
uniform sampler2D u_diagonals;
uniform float w;

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
    float d = texture(u_diagonals, v_texCoord).x;

    float pressure = mix(cell.x, (dot(p,c) + cell.y) / d, w);

    out_color = vec4(pressure, cell.y, 0.0, 0.0);
}