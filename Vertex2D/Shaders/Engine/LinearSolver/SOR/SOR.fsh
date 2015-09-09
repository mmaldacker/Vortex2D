#version 150

precision highp float;

in vec2 v_texCoord;
in vec2 v_texCoordxp;
in vec2 v_texCoordxn;
in vec2 v_texCoordyp;
in vec2 v_texCoordyn;

uniform sampler2D u_texture; // this is the pressure 
uniform sampler2D u_weights;
uniform float w;

const vec4 q = vec4(1.0);

out vec4 out_color;

void main()
{
    // cell.x is div and cell.y is pressure
    vec2 cell = texture(u_texture, v_texCoord).xy;

    vec4 p;
    p.x = texture(u_texture, v_texCoordxp).y;
	p.y = texture(u_texture, v_texCoordxn).y;
	p.z = texture(u_texture, v_texCoordyp).y;
	p.w = texture(u_texture, v_texCoordyn).y;

    vec4 c = texture(u_weights, v_texCoord);

    float pressure = mix(cell.y, (dot(p,c) + cell.x) / dot(q,c), w);

    out_color = vec4(cell.x, pressure, 0.0, 0.0);
}