#version 150

uniform sampler2D u_texture; // this is the velocity texture
uniform sampler2D u_obstacles;
uniform sampler2D u_obstacles_velocity;

in vec2 v_texCoord;
out vec4 out_color;

void main() 
{
    vec2  uv  = texture(u_texture, v_texCoord).xy;
    float uxp = textureOffset(u_texture, v_texCoord, ivec2(1,0)).x;
    float uxn = uv.x;
    float vyp = textureOffset(u_texture, v_texCoord, ivec2(0,1)).y;
    float vyn = uv.y;

    float c   = 1.0 - texture(u_obstacles, v_texCoord).x;
    float cxp = 1.0 - textureOffset(u_obstacles, v_texCoord, ivec2(2,0)).x;
    float cxn = 1.0 - textureOffset(u_obstacles, v_texCoord, ivec2(-2,0)).x;
    float cyp = 1.0 - textureOffset(u_obstacles, v_texCoord, ivec2(0,2)).x;
    float cyn = 1.0 - textureOffset(u_obstacles, v_texCoord, ivec2(0,-2)).x;

    float solid_uxp = textureOffset(u_obstacles_velocity, v_texCoord, ivec2(1,0)).x;
    float solid_uxn = textureOffset(u_obstacles_velocity, v_texCoord, ivec2(-1,0)).x;
    float solid_vyp = textureOffset(u_obstacles_velocity, v_texCoord, ivec2(0,1)).y;
    float solid_vyn = textureOffset(u_obstacles_velocity, v_texCoord, ivec2(0,-1)).y;

    float dx = textureSize(u_texture,0).x;
    float div = -(cxp * uxp - cxn * uxn + cyp * vyp - cyn * vyn) / dx;
          div += ((cxp-c)*solid_uxp - (cxn-c)*solid_uxn + (cyp-c)*solid_vyp - (cyn-c)*solid_vyn) / dx;

	//pressure, div, 0, 0
	out_color = vec4(0.0, div, 0.0, 0.0);
}