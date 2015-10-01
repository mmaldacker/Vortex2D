#version 150

precision highp float;

in vec2 v_texCoord;

uniform sampler2D u_texture; // this is the velocity texture
uniform sampler2D u_pressure; 
uniform sampler2D u_weights;
uniform sampler2D u_obstacles_velocity;

out vec4 out_color;

void main() 
{
    
	vec2 cell = texture(u_texture, v_texCoord).xy; 

    float pxp = textureOffset(u_pressure, v_texCoord, ivec2(1,0)).x;
    float pxn = textureOffset(u_pressure, v_texCoord, ivec2(-1,0)).x;
    float pyp = textureOffset(u_pressure, v_texCoord, ivec2(0,1)).x;
    float pyn = textureOffset(u_pressure, v_texCoord, ivec2(0,-1)).x;

    vec4 c = texture(u_weights, v_texCoord);
    
	vec2 pGrad = vec2(pxp-pxn, pyp-pyn);
    
    vec2 mask = vec2(1.0);
    vec2 obsV = vec2(0.0);
    
    if (c.x < 1.0)
    { 
        mask.x = 0.0; 
        obsV.x = textureOffset(u_obstacles_velocity, v_texCoord, ivec2(1,0)).x;
    }
    if (c.y < 1.0)
    { 
        mask.x = 0.0; 
        obsV.x = textureOffset(u_obstacles_velocity, v_texCoord, ivec2(-1,0)).x;
    }
    if (c.z < 1.0)
    { 
        mask.y = 0.0; 
        obsV.y = textureOffset(u_obstacles_velocity, v_texCoord, ivec2(0,1)).y;
    }
    if (c.w < 1.0)
    { 
        mask.y = 0.0; 
        obsV.y = textureOffset(u_obstacles_velocity, v_texCoord, ivec2(0,-1)).y;
    }
    
    // Enforce the free-slip boundary condition:
    vec2 new_cell = cell - (0.5 * textureSize(u_texture,0).x) * pGrad;
    out_color = vec4(mask * new_cell + obsV, 0.0, 0.0);
}