#version 150

precision highp float;

in vec2 v_texCoord;
in vec2 v_texCoordxp;
in vec2 v_texCoordxn;
in vec2 v_texCoordyp;
in vec2 v_texCoordyn;


uniform sampler2D u_texture; // this is the velocity texture
uniform sampler2D u_pressure; 
uniform sampler2D u_weights;
uniform sampler2D u_obstacles_velocity;

uniform vec2 h;

out vec4 out_color;

void main() 
{
    
	vec2 cell = texture(u_texture, v_texCoord).xy; 
    
	float pxp = texture(u_pressure, v_texCoordxp).x;
	float pxn = texture(u_pressure, v_texCoordxn).x;
	float pyp = texture(u_pressure, v_texCoordyp).x;
	float pyn = texture(u_pressure, v_texCoordyn).x;

    vec4 c = texture(u_weights, v_texCoord);
    
	vec2 pGrad = vec2(pxp-pxn, pyp-pyn);
    
    vec2 mask = vec2(1.0);
    vec2 obsV = vec2(0.0);
    
    if (c.x < 1.0)
    { 
        mask.x = 0.0; 
        obsV.x = texture(u_obstacles_velocity, v_texCoordxp).x;
    }
    if (c.y < 1.0)
    { 
        mask.x = 0.0; 
        obsV.x = texture(u_obstacles_velocity, v_texCoordxn).x;
    }
    if (c.z < 1.0)
    { 
        mask.y = 0.0; 
        obsV.y = texture(u_obstacles_velocity, v_texCoordyp).y;
    }
    if (c.w < 1.0)
    { 
        mask.y = 0.0; 
        obsV.y = texture(u_obstacles_velocity, v_texCoordyn).y;
    }
    
    // Enforce the free-slip boundary condition:
    vec2 new_cell = cell - (0.5 * h.x) * pGrad;
    out_color = vec4(mask * new_cell + obsV, 0.0, 0.0);
}