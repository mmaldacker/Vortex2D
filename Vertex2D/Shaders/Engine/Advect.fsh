#version 150

precision highp float;

in vec2 v_texCoord;
uniform sampler2D u_texture;
uniform sampler2D u_velocity;

uniform float delta;
uniform vec2 h;

const vec2 off = vec2(.5, .5);

out vec4 out_color;

vec2 bilerp(sampler2D u_texture, vec2 xy)
{
    vec4 ij;
    ij.xy = floor(xy);
    ij.zw = ij.xy + 1.0;
    vec2 f = xy - ij.xy;
    
    vec4 st = (ij + vec4(off,off)) / vec4(h,h);
    
    vec2 t11 = texture(u_texture, st.xy).xy;
    vec2 t21 = texture(u_texture, st.xw).xy;
    vec2 t12 = texture(u_texture, st.zy).xy;
    vec2 t22 = texture(u_texture, st.zw).xy;
    
    return mix(mix(t11,t21,f.y),mix(t12,t22,f.y),f.x);
}

void main(void) 
{
    vec2 real_coord = gl_FragCoord.xy - off;
    
    vec2 stepBackCoords = real_coord - delta * texture(u_velocity, v_texCoord).xy;

    vec2 stepForwardCoords = stepBackCoords + delta * bilerp(u_velocity, stepBackCoords);

    stepBackCoords = stepBackCoords + (stepBackCoords - stepForwardCoords) * 0.5;

    out_color = vec4(bilerp(u_texture, stepBackCoords), 0.0, 0.0);
}