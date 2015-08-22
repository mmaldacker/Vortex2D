#version 150

precision highp float;

uniform mediump vec4 u_Colour;

void main()
{
	gl_FragColor = u_Colour;
}
