#version 150

precision highp float;

in vec2 v_texCoord;

uniform sampler2D u_texture;
uniform sampler2D u_velocity;

out vec4 out_color;

const vec2 off = vec2(0.5);

void main()
{
    float s = sign(texture(u_texture, v_texCoord).x);

    float wxp = textureOffset(u_texture, v_texCoord, ivec2(1,0)).x;
    float wxn = textureOffset(u_texture, v_texCoord, ivec2(-1,0)).x;
    float wyp = textureOffset(u_texture, v_texCoord, ivec2(0,1)).x;
    float wyn = textureOffset(u_texture, v_texCoord, ivec2(0,-1)).x;

    vec2 w = normalize(vec2(wxp-wxn, wyp-wyn));

    vec2 real_coord = gl_FragCoord.xy - off;

    vec2 coords = real_coord - s * w;

    vec2 stepBackwardsCoords = (coords + off) / textureSize(u_velocity, 0).xy;

    out_color = vec4(texture(u_velocity, stepBackwardsCoords).xy, 0.0, 0.0);
}
