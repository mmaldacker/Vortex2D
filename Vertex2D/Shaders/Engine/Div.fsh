#version 150

precision highp float;

in vec2 v_texCoord;

uniform sampler2D u_texture; // this is the velocity texture
uniform sampler2D u_obstacles;
uniform sampler2D u_obstacles_velocity;

out vec4 out_color;

void main() 
{
    vec2  uv  = texture(u_texture, v_texCoord).xy;
    float uxp = textureOffset(u_texture, v_texCoord, ivec2(1,0)).x;
    float uxn = uv.x;
    float vyp = textureOffset(u_texture, v_texCoord, ivec2(0,1)).y;
    float vyn = uv.y;

    float cxp = textureOffset(u_obstacles, v_texCoord, ivec2(2,0)).x;
    float cxn = textureOffset(u_obstacles, v_texCoord, ivec2(-2,0)).x;
    float cyp = textureOffset(u_obstacles, v_texCoord, ivec2(0,2)).x;
    float cyn = textureOffset(u_obstacles, v_texCoord, ivec2(0,-2)).x;

    if (cxp > 0.0)
    { 
        uxp = -uv.x + textureOffset(u_obstacles_velocity, v_texCoord, ivec2(1,0)).x;
    }
    if (cxn > 0.0)
    { 
        uxn = -uv.x + textureOffset(u_obstacles_velocity, v_texCoord, ivec2(0,0)).x;
    }
    if (cyp > 0.0)
    { 
        vyp = -uv.y + textureOffset(u_obstacles_velocity, v_texCoord, ivec2(0,1)).y;
    }
    if (cyn > 0.0)
    { 
        vyn = -uv.y + textureOffset(u_obstacles_velocity, v_texCoord, ivec2(0,0)).y;
    }

    float div = -0.5 * ((1.0 - cxp) * uxp - (1.0 - cxn) * uxn + (1.0 - cyp) * vyp - (1.0 - cyn) * vyn) / textureSize(u_texture,0).x;

	//pressure, div, 0, 0
	out_color = vec4(0.0, div, 0.0, 0.0);
}