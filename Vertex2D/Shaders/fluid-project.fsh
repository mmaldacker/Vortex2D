precision highp float;

varying mediump vec2 v_texCoord;
varying mediump vec2 v_texCoordxp;
varying mediump vec2 v_texCoordxn;
varying mediump vec2 v_texCoordyp;
varying mediump vec2 v_texCoordyn;


uniform sampler2D u_texture; // this is the velocity texture
uniform sampler2D u_pressure; // this is the pressure texture as solved by jacobi iterations
uniform sampler2D u_weights;
uniform sampler2D u_obstacles_velocity;

uniform vec2 h;

void main() 
{
    
	vec2 cell = texture2D(u_texture, v_texCoord).xy; 
    
    //float p   = texture2D(u_pressure, v_texCoord).y;
	float pxp = texture2D(u_pressure, v_texCoordxp).y;
	float pxn = texture2D(u_pressure, v_texCoordxn).y; 
	float pyp = texture2D(u_pressure, v_texCoordyp).y;
	float pyn = texture2D(u_pressure, v_texCoordyn).y;

    vec4 c = texture2D(u_weights, v_texCoord);
    
	vec2 pGrad = vec2(pxp-pxn, pyp-pyn);
    
    vec2 mask = vec2(1.0);
    vec2 obsV = vec2(0.0);
    
    if (c.x < 1.0)
    { 
        mask.x = 0.0; 
        obsV.x = texture2D(u_obstacles_velocity, v_texCoordxp).x;
    }
    if (c.y < 1.0)
    { 
        mask.x = 0.0; 
        obsV.x = texture2D(u_obstacles_velocity, v_texCoordxn).x;
    }
    if (c.z < 1.0)
    { 
        mask.y = 0.0; 
        obsV.y = texture2D(u_obstacles_velocity, v_texCoordyp).y;
    }
    if (c.w < 1.0)
    { 
        mask.y = 0.0; 
        obsV.y = texture2D(u_obstacles_velocity, v_texCoordyn).y;
    }
    
    // Enforce the free-slip boundary condition:
    vec2 new_cell = cell - (0.5 * h.x) * pGrad;
    gl_FragColor = vec4(mask * new_cell + obsV, 0.0, 0.0);  
}