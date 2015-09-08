#version 150

precision highp float;

in vec2 v_texCoord;
uniform sampler2D u_texture;
uniform sampler2D u_velocity;

uniform float delta;
uniform vec2 h;

const vec2 off = vec2(.5, .5);

out vec4 out_color;

void main(void) 
{
    vec2 real_coord = gl_FragCoord.xy - off;
    
    vec2 stepBackCoords = real_coord - delta * texture(u_velocity, v_texCoord).xy;

    vec2 stepForwardCoords = stepBackCoords + delta * texture(u_velocity, stepBackCoords).xy;
    
    stepBackCoords = stepBackCoords + (stepBackCoords - stepForwardCoords) * 0.5;
    
    out_color = texture(u_texture, (stepBackCoords+off)/h);

}