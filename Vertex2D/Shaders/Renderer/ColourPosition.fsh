#version 150

precision highp float;

in vec4 v_Colour;

uniform mediump vec4 u_Colour;

out vec4 out_color;

void main()
{
	out_color = v_Colour * u_Colour;
}
