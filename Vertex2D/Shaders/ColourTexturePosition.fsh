#version 150

precision highp float;

in mediump vec2 v_TexCoords;
uniform sampler2D u_Texture;
uniform mediump vec4 u_Colour;

out vec4 out_color;

void main()
{
	out_color = texture(u_Texture, v_TexCoords) * u_Colour;
}
