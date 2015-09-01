#version 150

precision highp float;

in vec2 v_texCoord;
uniform sampler2D u_texture;
uniform sampler2D u_velocity;
uniform sampler2D u_obstacles;

uniform float delta;
uniform vec2 h;
uniform vec2 xy_min;
uniform vec2 xy_max;

const vec2 off = vec2(.5, .5);

out vec4 out_color;

void main(void) 
{
    if(texture(u_obstacles, v_texCoord).x > 0.0)
    {
        out_color = vec4(0.0);
        return;
    }
    
    vec2 real_coord = gl_FragCoord.xy - off;
    
    vec2 stepBackCoords = real_coord - delta * texture(u_velocity, v_texCoord).xy;
    stepBackCoords = clamp(stepBackCoords, xy_min, xy_max);

    vec2 stepForwardCoords = stepBackCoords + delta * texture(u_velocity, stepBackCoords).xy;
    stepForwardCoords = clamp(stepForwardCoords, xy_min, xy_max);
    
    stepBackCoords = stepBackCoords + (stepBackCoords - stepForwardCoords) * 0.5;
    
    out_color = texture(u_texture, (stepBackCoords+off)/h);

}