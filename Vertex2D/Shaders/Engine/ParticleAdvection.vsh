#version 150

in vec2 a_Position;

uniform sampler2D u_velocity;

uniform float delta;

out vec2 outPosition;

vec2 bilerp(sampler2D u_texture, vec2 xy)
{
    ivec4 ij;
    ij.xy = ivec2(floor(xy));
    ij.zw = ij.xy + 1;
    vec2 f = xy - ij.xy;

    vec2 t11 = texelFetch(u_texture, ij.xy, 0).xy;
    vec2 t21 = texelFetch(u_texture, ij.xw, 0).xy;
    vec2 t12 = texelFetch(u_texture, ij.zy, 0).xy;
    vec2 t22 = texelFetch(u_texture, ij.zw, 0).xy;

    return mix(mix(t11,t21,f.y),mix(t12,t22,f.y),f.x);
}

void main()
{
    vec2 local_Position = a_Position * 0.5;

    vec2 q1 = local_Position + 0.5 * delta * bilerp(u_velocity, local_Position);
    vec2 q2 = q1 + delta * bilerp(u_velocity, q1);

    outPosition = q2*2.0;
}
