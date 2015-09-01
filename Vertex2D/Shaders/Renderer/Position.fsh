#version 150

precision highp float;

uniform vec4 u_Colour;

out vec4 out_color;

void main()
{
	out_color = u_Colour;
}
