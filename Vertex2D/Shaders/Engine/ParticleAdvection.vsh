#version 150

precision highp float;

in vec2 a_Position;

uniform sampler2D u_velocity;

uniform float delta;

const vec2 off = vec2(0.5);

out vec2 outPosition;

vec2 bilerp(sampler2D u_texture, vec2 xy)
{
    vec4 ij;
    ij.xy = floor(xy);
    ij.zw = ij.xy + 1.0;
    vec2 f = xy - ij.xy;

    vec2 h = textureSize(u_velocity, 0).xy;
    vec4 st = (ij + vec4(off,off)) / vec4(h,h);

    vec2 t11 = texture(u_texture, st.xy).xy;
    vec2 t21 = texture(u_texture, st.xw).xy;
    vec2 t12 = texture(u_texture, st.zy).xy;
    vec2 t22 = texture(u_texture, st.zw).xy;

    return mix(mix(t11,t21,f.y),mix(t12,t22,f.y),f.x);
}

void main()
{
    vec2 h = vec2(2.0) * textureSize(u_velocity, 0).xy;
    vec2 texPosition = (a_Position + off)/h;

    vec2 q1 = texPosition + 0.5 * delta * texture(u_velocity, texPosition).xy;
    vec2 q2 = q1 + delta * bilerp(u_velocity, q1);

    outPosition = (h*q2)-off;
}
