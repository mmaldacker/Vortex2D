#version 150

uniform sampler2D u_texture;
uniform sampler2D u_velocity;
uniform float delta;

in vec2 v_texCoord;
out vec4 out_color;

void main(void) 
{

    vec2 stepBackCoords = gl_FragCoord.xy - 0.5 - delta * texture(u_velocity, v_texCoord).xy;
    vec2 stepForwardCoords = stepBackCoords + delta * texture(u_velocity, stepBackCoords).xy;
    
    stepBackCoords = stepBackCoords + (stepBackCoords - stepForwardCoords) * 0.5;
    
    out_color = texture(u_texture, (stepBackCoords+0.5)/textureSize(u_texture, 0));

}