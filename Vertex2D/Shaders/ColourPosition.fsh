precision highp float;

varying mediump vec4 v_Colour;

uniform mediump vec4 u_Colour;

void main()
{
	gl_FragColor = v_Colour * u_Colour;
}
