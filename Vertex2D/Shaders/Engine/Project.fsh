#version 150

precision highp float;

in vec2 v_texCoord;

uniform sampler2D u_texture; // this is the velocity texture
uniform sampler2D u_pressure; 
uniform sampler2D u_obstacles;
uniform sampler2D u_obstacles_velocity;
uniform float delta;

out vec4 out_color;

void main() 
{
    
	vec2 cell = texture(u_texture, v_texCoord).xy; 

    float pxp = textureOffset(u_pressure, v_texCoord, ivec2(1,0)).x;
    float pxn = textureOffset(u_pressure, v_texCoord, ivec2(-1,0)).x;
    float pyp = textureOffset(u_pressure, v_texCoord, ivec2(0,1)).x;
    float pyn = textureOffset(u_pressure, v_texCoord, ivec2(0,-1)).x;

    float cxp = textureOffset(u_obstacles, v_texCoord, ivec2(2,0)).x;
    float cxn = textureOffset(u_obstacles, v_texCoord, ivec2(-2,0)).x;
    float cyp = textureOffset(u_obstacles, v_texCoord, ivec2(0,2)).x;
    float cyn = textureOffset(u_obstacles, v_texCoord, ivec2(0,-2)).x;
    
	vec2 pGrad = vec2(pxp-pxn, pyp-pyn);
    
    vec2 mask = vec2(1.0);
    vec2 obsV = vec2(0.0);
    
    if (cxp > 0.0)
    { 
        mask.x = 0.0; 
        obsV.x = textureOffset(u_obstacles_velocity, v_texCoord, ivec2(1,0)).x;
    }
    if (cxn > 0.0)
    { 
        mask.x = 0.0; 
        obsV.x = textureOffset(u_obstacles_velocity, v_texCoord, ivec2(-1,0)).x;
    }
    if (cyp > 0.0)
    { 
        mask.y = 0.0; 
        obsV.y = textureOffset(u_obstacles_velocity, v_texCoord, ivec2(0,1)).y;
    }
    if (cyn > 0.0)
    { 
        mask.y = 0.0; 
        obsV.y = textureOffset(u_obstacles_velocity, v_texCoord, ivec2(0,-1)).y;
    }

    float dx = 1.0;
    vec2 new_cell = cell - delta * pGrad / dx;
    out_color = vec4(mask * new_cell + obsV, 0.0, 0.0);
}