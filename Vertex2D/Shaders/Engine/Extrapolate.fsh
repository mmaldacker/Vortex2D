#version 150

precision highp float;

in vec2 v_texCoord;

uniform sampler2D u_texture;
uniform sampler2D u_velocity;

out vec4 out_color;

void main()
{
    float s = sign(texture(u_texture, v_texCoord).x);

    float wxp = textureOffset(u_texture, v_texCoord, ivec2(1,0)).x;
    float wxn = textureOffset(u_texture, v_texCoord, ivec2(-1,0)).x;
    float wyp = textureOffset(u_texture, v_texCoord, ivec2(0,1)).x;
    float wyn = textureOffset(u_texture, v_texCoord, ivec2(0,-1)).x;

    vec2 w = normalize(vec2(wxp-wxn, wyp-wyn));

    float dx = 1.0 / textureSize(u_texture, 0).x;

    vec2 coords = gl_FragCoord.xy - 0.5 - s * w;

    vec2 stepBackwardsCoords = (coords + 0.5) * dx;

    out_color = vec4(texture(u_velocity, stepBackwardsCoords).xy, 0.0, 0.0);
}
