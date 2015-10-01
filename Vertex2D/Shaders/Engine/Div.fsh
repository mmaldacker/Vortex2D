#version 150

precision highp float;

in vec2 v_texCoord;

uniform sampler2D u_texture; // this is the velocity texture
uniform sampler2D u_weights;
uniform sampler2D u_obstacles_velocity;

out vec4 out_color;

void main() 
{
    vec4 weights = texture(u_weights, v_texCoord);

    vec2  uv  = texture(u_texture, v_texCoord).xy;
    float uxp = textureOffset(u_texture, v_texCoord, ivec2(1,0)).x;
    float uxn = textureOffset(u_texture, v_texCoord, ivec2(-1,0)).x;
    float vyp = textureOffset(u_texture, v_texCoord, ivec2(0,1)).y;
    float vyn = textureOffset(u_texture, v_texCoord, ivec2(0,-1)).y;

    if (weights.x < 1.0)
    { 
        uxp = -uv.x + textureOffset(u_obstacles_velocity, v_texCoord, ivec2(1,0)).x;
    }
    if (weights.y < 1.0)
    { 
        uxn = -uv.x + textureOffset(u_obstacles_velocity, v_texCoord, ivec2(-1,0)).x;
    }
    if (weights.z < 1.0)
    { 
        vyp = -uv.y + textureOffset(u_obstacles_velocity, v_texCoord, ivec2(0,1)).y;
    }
    if (weights.w < 1.0)
    { 
        vyn = -uv.y + textureOffset(u_obstacles_velocity, v_texCoord, ivec2(0,-1)).y;
    }

    float div = -0.5 * (weights.x * uxp - weights.y * uxn + weights.z * vyp - weights.w * vyn) / textureSize(u_texture,0).x;

	//pressure, div, 0, 0
	out_color = vec4(0.0, div, 0.0, 0.0);
}