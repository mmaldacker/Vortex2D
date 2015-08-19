precision highp float;

varying mediump vec2 v_texCoord;
varying mediump vec2 v_texCoordxp;
varying mediump vec2 v_texCoordxn;
varying mediump vec2 v_texCoordyp;
varying mediump vec2 v_texCoordyn;


uniform sampler2D u_texture; // this is the velocity texture
uniform sampler2D u_weights;
uniform sampler2D u_obstacles_velocity;
uniform vec2 h;

void main() 
{
    vec4 weights = texture2D(u_weights, v_texCoord);

    vec2  uv  = texture2D(u_texture, v_texCoord).xy;
	float uxp = texture2D(u_texture, v_texCoordxp).x;
	float uxn = texture2D(u_texture, v_texCoordxn).x;
	float vyp = texture2D(u_texture, v_texCoordyp).y;
	float vyn = texture2D(u_texture, v_texCoordyn).y;

    if (weights.x < 1.0)
    { 
        uxp = -uv.x + texture2D(u_obstacles_velocity, v_texCoordxp).x;
    }
    if (weights.y < 1.0)
    { 
        uxn = -uv.x + texture2D(u_obstacles_velocity, v_texCoordxn).x;
    }
    if (weights.z < 1.0)
    { 
        vyp = -uv.y + texture2D(u_obstacles_velocity, v_texCoordyp).y;
    }
    if (weights.w < 1.0)
    { 
        vyn = -uv.y + texture2D(u_obstacles_velocity, v_texCoordyn).y;
    }

	float div = -0.5 * (weights.x * uxp - weights.y * uxn + weights.z * vyp - weights.w * vyn) / h.x;

	//div, pressure, 0, 0
	gl_FragColor = vec4(div, 0.0, 0.0, 0.0);
}