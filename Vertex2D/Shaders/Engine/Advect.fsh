#version 150

uniform sampler2D u_texture;
uniform sampler2D u_velocity;
uniform float delta;

in vec2 v_texCoord;
out vec4 out_color;

vec2 bilerp(sampler2D u_texture, vec2 xy)
{
    vec4 ij;
    ij.xy = floor(xy);
    ij.zw = ij.xy + 1.0;
    vec2 f = xy - ij.xy;

    vec2 h = textureSize(u_texture, 0);
    vec4 st = (ij + vec4(0.5)) / vec4(h,h);
    
    vec2 t11 = texture(u_texture, st.xy).xy;
    vec2 t21 = texture(u_texture, st.xw).xy;
    vec2 t12 = texture(u_texture, st.zy).xy;
    vec2 t22 = texture(u_texture, st.zw).xy;
    
    return mix(mix(t11,t21,f.y),mix(t12,t22,f.y),f.x);
}

void main(void) 
{
    vec2 stepBackCoords = gl_FragCoord.xy - 0.5 - delta * texture(u_velocity, v_texCoord).xy;
    vec2 stepForwardCoords = stepBackCoords + delta * bilerp(u_velocity, stepBackCoords);

    stepBackCoords = stepBackCoords + (stepBackCoords - stepForwardCoords) * 0.5;

    out_color = vec4(bilerp(u_texture, stepBackCoords), 0.0, 0.0);
}