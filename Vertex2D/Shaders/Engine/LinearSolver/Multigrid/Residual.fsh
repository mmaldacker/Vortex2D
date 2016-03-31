#version 150

precision highp float;

in vec2 v_texCoord;

uniform sampler2D u_texture;
uniform sampler2D u_weights;
uniform sampler2D u_diagonals;

out vec4 colour_out;

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

    float residual = dot(p,c) - d * cell.x + cell.y;
    colour_out = vec4(residual, cell.y, 0.0, 0.0);
}