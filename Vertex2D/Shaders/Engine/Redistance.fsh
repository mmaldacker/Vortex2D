#version 150

precision highp float;

in vec2 v_texCoord;

uniform sampler2D u_texture;

uniform float delta;
uniform vec2 h; //FIXME use textureSize

out vec4 out_color;

vec2 bilerp(sampler2D u_texture, vec2 xy)
{
    vec4 ij;
    ij.xy = floor(xy);
    ij.zw = ij.xy + 1.0;
    vec2 f = xy - ij.xy;

    vec4 st = (ij + 0.5) / vec4(h,h);

    vec2 t11 = texture(u_texture, st.xy).xy;
    vec2 t21 = texture(u_texture, st.xw).xy;
    vec2 t12 = texture(u_texture, st.zy).xy;
    vec2 t22 = texture(u_texture, st.zw).xy;

    return mix(mix(t11,t21,f.y),mix(t12,t22,f.y),f.x);
}

void main()
{
    float s = sign(texture(u_texture, v_texCoord).x);

    float wxp = textureOffset(u_texture, v_texCoord, ivec2(1,0)).x;
    float wxn = textureOffset(u_texture, v_texCoord, ivec2(-1,0)).x;
    float wyp = textureOffset(u_texture, v_texCoord, ivec2(0,1)).x;
    float wyn = textureOffset(u_texture, v_texCoord, ivec2(0,-1)).x;

    vec2 w = normalize(vec2(wxp-wxn, wyp-wyn));

    vec2 stepBackCoords = gl_FragCoord.xy - 0.5 - delta * s * w;

    out_color = vec4(bilerp(u_texture, stepBackCoords).x + delta * s, 0.0, 0.0, 0.0);
}
